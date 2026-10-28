#include "audio.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sai.h"
#include "ff.h"
#include "app_files.h"
#include "fatfs.h"
#include "pcm5122.h"
#include "main.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define AUDIO_STAGE_START  0
#define AUDIO_STAGE_HALFIT  1
#define AUDIO_STAGE_FULLIT  2

#define AUDIO_BUFF_SIZE   512 * 16

uint8_t data_i2s[AUDIO_BUFF_SIZE];
uint8_t data_i2s_tmp[AUDIO_BUFF_SIZE + 4];

uint8_t file_system_err = 0;

FIL wavFile;
char file_ptr[64] = {0,};
static uint8_t is_cycle_play = 0;
static uint8_t first_play = 0;

static uint8_t presc = 2;

static Files files;
static int current_file_index = 0;

uint8_t sai_dma_state = 0U;

extern SemaphoreHandle_t xSemaphore_sdcard;
extern SAI_HandleTypeDef hsai_BlockB2;
extern TaskHandle_t audioTaskHandle;
char path[4 + MAX_FILE_PATH_LENGTH + 1];

static const char *GetCurrentFilePath(void) 
{
  snprintf(path, sizeof(path), "SD:/%s", files.files[current_file_index]);
  return path;
}

uint8_t play_record(uint8_t stage)
{
  FRESULT fr = FR_NOT_READY;
	UINT bytesRead = 0;
  
  static uint32_t data_lenght = 0;
  static const char* path_ptr;
  static uint8_t is_aligned = 1;
  
  xSemaphoreTake(xSemaphore_sdcard, portMAX_DELAY);
	if(first_play == 0)
  {
    if(is_cycle_play)
    {
      path_ptr = GetCurrentFilePath();
    }
    else
    {
      path_ptr = file_ptr;
    }
    
		fr = f_open(&wavFile, path_ptr, FA_READ);

    uint16_t audio_format = 0;
    uint16_t ch_num = 0;
    uint32_t freq = 0;
    uint32_t byte_rate = 0;
    uint32_t baudrate_set = 0;
    uint32_t mode_set = 0;
    uint8_t subchunk_header[4];
    uint32_t subchunk_size;
    uint8_t cmp;
		if (fr == FR_OK)
		{
      f_read(&wavFile, data_i2s_tmp, 256, &bytesRead);
		  f_lseek(&wavFile, 12);
      
      do
      {
        fr |= f_read(&wavFile, subchunk_header, 4, &bytesRead);
        fr |= f_read(&wavFile, &subchunk_size, 4, &bytesRead);

        cmp = 0;
        if(!strncmp((char*)subchunk_header, "fmt ", 4))
        {
          fr |= f_read(&wavFile, data_i2s_tmp, subchunk_size, &bytesRead);
          audio_format   = *(uint16_t*)&data_i2s_tmp[0];
          ch_num   = *(uint16_t*)&data_i2s_tmp[22 - 20];
          freq = *(uint32_t*)&data_i2s_tmp[24 - 20];

          byte_rate = *(uint32_t*)&data_i2s_tmp[28 - 20];
          data_lenght = *(uint16_t*)&data_i2s_tmp[34 - 20];
        }
        else if (!strncmp((char*)subchunk_header, "data", 4))
        {
         cmp = 1;
        }
        else
        {
          f_lseek(&wavFile, f_tell(&wavFile) + subchunk_size);
        }
      } while ((fr == FR_OK) && !cmp);
      
      is_aligned = !(f_tell(&wavFile) % 4);
      if(!is_aligned)
      {
        f_lseek(&wavFile, f_tell(&wavFile) - 2);
      }
      
      switch (data_lenght)
      {
        case 16:
        baudrate_set = SAI_PROTOCOL_DATASIZE_16BIT;
        presc = 2;
        break;
        case 24:
        baudrate_set = SAI_PROTOCOL_DATASIZE_24BIT;
        presc = 4;
        break;
        case 32:
        baudrate_set = SAI_PROTOCOL_DATASIZE_32BIT;
        presc = 4;
        break;
        default:
        baudrate_set = 0;
        break;
      }
      // PCM5122_SetBaudrate(data_lenght);
      if (ch_num == 1)
      {
        mode_set = SAI_MONOMODE;
      }
      else
      {
        mode_set = SAI_STEREOMODE;
      }
      SAI_SetAudioFrBr (freq, baudrate_set, mode_set);
		}
		first_play = 1;
	}
  
	// f_read(&wavFile, data, data_size, &bytesRead);
  uint8_t * ptr = data_i2s;
  uint32_t d_read = 0;
  if(stage == AUDIO_STAGE_START)
  {
    d_read = AUDIO_BUFF_SIZE;
  }
  else if (stage == AUDIO_STAGE_HALFIT)
  {
    d_read = AUDIO_BUFF_SIZE / 2;
  }
  else if (stage == AUDIO_STAGE_FULLIT)
  {
    d_read = AUDIO_BUFF_SIZE / 2;
    ptr = data_i2s + AUDIO_BUFF_SIZE / 2;
  }
  
  if(is_aligned)
  {
    fr = f_read(&wavFile, ptr, d_read, &bytesRead);
  }
  else
  {
    fr = f_read(&wavFile, data_i2s_tmp, d_read, &bytesRead);
    memcpy(ptr, data_i2s_tmp + 2, d_read);
    if(stage == AUDIO_STAGE_HALFIT)
    {
      data_i2s[AUDIO_BUFF_SIZE - 2] = data_i2s_tmp[0];
      data_i2s[AUDIO_BUFF_SIZE - 1] = data_i2s_tmp[1];
    }
    else if(stage == AUDIO_STAGE_FULLIT)
    {
      data_i2s[AUDIO_BUFF_SIZE/2 - 2] = data_i2s_tmp[0];
      data_i2s[AUDIO_BUFF_SIZE/2 - 1] = data_i2s_tmp[1];
    }
  }

  if(fr==FR_OK)
  {
    if(bytesRead < d_read)
    {
      f_close(&wavFile);
      xSemaphoreGive(xSemaphore_sdcard);
      if(is_cycle_play)
      {
        current_file_index = (current_file_index + 1) % files.count;
        first_play = 0;
        return 0;
      }
      else
      {
        first_play = 0;
        return 2;
      }
	  }
  }
  else// if (fr == FR_DISK_ERR)
  {
    f_close(&wavFile);
    xSemaphoreGive(xSemaphore_sdcard);
    memset(&wavFile, 0x00, sizeof(FIL));
    first_play = 0;
    file_system_err = 1;
    return 2;
  }
  
  xSemaphoreGive(xSemaphore_sdcard);
	return 1;
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  BaseType_t xYeldReq = xTaskResumeFromISR (audioTaskHandle);
  sai_dma_state = 1;
  portYIELD_FROM_ISR(xYeldReq);
}

void HAL_SAI_TxCpltCallback (SAI_HandleTypeDef *hsai)
{
  BaseType_t xYeldReq = xTaskResumeFromISR (audioTaskHandle);
  sai_dma_state = 2;
  portYIELD_FROM_ISR(xYeldReq);
}

void set_cycleplay(uint8_t flag)
{
  is_cycle_play = flag;
}

void PauseAudio (void)
{
  HAL_SAI_DMAPause(&hsai_BlockB2);
}

int32_t get_audio_lenght (char* f_ptr)
{
  uint32_t freq = 0;
  uint16_t ch_num = 0;
  uint32_t data_lenght = 0;
  uint32_t data_size = 0;
  int32_t audio_lenght = 0;
  UINT bytesRead = 0;
  uint8_t data[44];

  HAL_SAI_DMAStop(&hsai_BlockB2);
  xSemaphoreTake(xSemaphore_sdcard, portMAX_DELAY);
  f_close(&wavFile);
  if(f_open(&wavFile, f_ptr, FA_READ) != FR_OK)
  {
    xSemaphoreGive(xSemaphore_sdcard);
    return -1;
  }
  
	// f_lseek(&wavFile, 44);
  f_read(&wavFile, data, 44, &bytesRead);
  f_close(&wavFile);
  xSemaphoreGive(xSemaphore_sdcard);
  freq = *(uint32_t*)&data[24];
  data_lenght = *(uint16_t*)&data[34];
  ch_num   = *(uint16_t*)&data[22];
  data_size   = (*(uint32_t*)&data[4])-44;
  
  audio_lenght = data_size / freq / ch_num /(data_lenght / 8);
  return audio_lenght;
}

float get_audio_progress (void)
{
  if(f_size(&wavFile)!= 0)
  {
    return (float)f_tell(&wavFile) / (float)f_size(&wavFile);
  }
  else
  {
    return -1.0f;
  }
}

void PlayAudioByFilename (char* f_ptr)
{
  HAL_SAI_DMAStop(&hsai_BlockB2);
  xSemaphoreTake(xSemaphore_sdcard, portMAX_DELAY);
  f_close(&wavFile);
  xSemaphoreGive(xSemaphore_sdcard);
  snprintf(file_ptr, sizeof(file_ptr), "SD:/%s", f_ptr);
  sai_dma_state = 0;
  first_play = 0;
  set_cycleplay(0);
  
  play_record(AUDIO_STAGE_START);
  HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t*)data_i2s, AUDIO_BUFF_SIZE/presc);
}

void PlayCycleAudio (void)
{
  if(is_cycle_play == 0)
  {
    FindWavFiles("SD:/", &files);
  
    sai_dma_state = 0;
    set_cycleplay(1);
    
    play_record(AUDIO_STAGE_START);
    HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t*)data_i2s, AUDIO_BUFF_SIZE/presc);
    HAL_SAI_DMAPause(&hsai_BlockB2);
  }
  HAL_SAI_DMAResume(&hsai_BlockB2);
}

void PlayNext (void)
{
  xSemaphoreTake(xSemaphore_sdcard, portMAX_DELAY);
  f_close(&wavFile);
  xSemaphoreGive(xSemaphore_sdcard);
  current_file_index = (current_file_index + 1) % files.count;
	first_play = 0;
  HAL_SAI_DMAStop(&hsai_BlockB2);
  play_record(AUDIO_STAGE_START);
  HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t*)data_i2s, AUDIO_BUFF_SIZE/presc);
}

void AudioProcess (void)
{
  uint8_t state = 0;
  if (sai_dma_state != 0)
  {
    state = play_record(sai_dma_state);
    if ( state == 0)
    {
      HAL_SAI_DMAStop(&hsai_BlockB2);
      play_record(AUDIO_STAGE_START);
      HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t*)data_i2s, AUDIO_BUFF_SIZE/presc);
    }
    else if (state == 2)
    {
      HAL_SAI_DMAStop(&hsai_BlockB2);
    }
    sai_dma_state = 0;
  }
}
