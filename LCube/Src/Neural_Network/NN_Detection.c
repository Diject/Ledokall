
#include "NN_Detection.h"

uint8_t detection_result[4] = {0xff, 0xff, 0xff, 0xff};

detection_command command_data[NN_DETECTION_COMMANDS_COUNT] = {0};

//Отношение слова к позиции в голосовой команде
const uint8_t detection_triggerStep[AI_VOICECOMMANDSNN_OUT_1_SIZE] = {0, 2, 2, 3, 3, 3, 3, 1, 2, 3, 3, 3, 3, 3};
uint32_t detection_start_tick = 0;
uint32_t detection_last_detection_tick = 0;
uint32_t detection_last_successful_detection_tick = 0;

void NN_Detect_Init()
{
	command_data[0].label = 0x06010700; //Ледокол Пожалуйста Включи Время
	command_data[0].action = &NN_Action_ShowTimeScreen;
	command_data[1].label = 0x03010700; //Ледокол Пожалуйста Включи Спектрограмму
	command_data[1].action = &NN_Action_ShowMicSpectrogramScreen;
	command_data[2].label = 0x04010700; //Ледокол Пожалуйста Включи Температуру
	command_data[2].action = &NN_Action_ShowTemperatureScreen;
	command_data[3].label = 0x05010700; //Ледокол Пожалуйста Включи Влажность
	command_data[3].action = &NN_Action_ShowHumidityScreen;
	command_data[4].label = 0x06020700; //Ледокол Пожалуйста Произнеси Время
	command_data[4].action = &NN_Action_SayTime;
	command_data[5].label = 0x04020700; //Ледокол Пожалуйста Произнеси Температуру
	command_data[5].action = &NN_Action_SayTemperature;
	command_data[6].label = 0x05020700; //Ледокол Пожалуйста Произнеси Влажность
	command_data[6].action = &NN_Action_SayHumidity;

	command_data[7].label = 0x0A080700; //Ледокол Пожалуйста Выключи Звук
	command_data[7].action = &NN_Action_MuteSound;
	command_data[8].label = 0x0A010700; //Ледокол Пожалуйста Включи Звук
	command_data[8].action = &NN_Action_UnMuteSound;
	command_data[9].label = 0x0B080700; //Ледокол Пожалуйста Выключи Микрофон
	command_data[9].action = &NN_Action_MuteMicrophone;
	command_data[10].label = 0x0B010700; //Ледокол Пожалуйста Включи Микрофон
	command_data[10].action = &NN_Action_UnMuteMicrophone;

	command_data[11].label = 0x0D080700; //Ледокол Пожалуйста Выключи Людей
	command_data[11].action = &NN_Action_Joke;
}


void NN_Detect_Process(int32_t nn_result)
{
	if ((nn_result != -1) && (nn_result != (AI_VOICECOMMANDSNN_OUT_1_SIZE - 1)))
	{
		detection_last_detection_tick = get_device_counter();
		if (detection_triggerStep[nn_result] == 0) //Старт-слово
		{
			*(uint32_t *)detection_result = NN_DETECTION_RESET_STATE;
			detection_result[0] = nn_result;
			detection_start_tick = get_device_counter();
			detection_last_successful_detection_tick = get_device_counter();
			return;
		}
		//Не выполнять, если не прошло время блокировки
		if ((get_device_counter() - detection_last_successful_detection_tick) < NN_DETECTION_LOCK_TIME)
		{
			return;
		}
		//Не выполнять, если нарушена очередность
		for (int32_t i = detection_triggerStep[nn_result] - 1; i >= 0; i--)
		{
			if (detection_result[i] == 0xff)
			{
				return;
			}
		}
		detection_result[detection_triggerStep[nn_result]] = nn_result;
		detection_last_successful_detection_tick = get_device_counter();
//		if (detection_result[0] == 0 && detection_result[1] == 7)
//		{
//			device.neuralNetworkId = 1;
//		}
	}
	//Сбросить, если вышло время
	if ((get_device_counter() - detection_start_tick) > NN_DETECTION_TIME_LIMIT)
	{
		if (device.neuralNetworkId)
		{
			NN_ClearAudioBuffer();
			device.neuralNetworkId = 0;
		}
		*(uint32_t *)detection_result = NN_DETECTION_RESET_STATE;
		return;
	}
	//Выполенение команды
	if (((get_device_counter() - detection_last_detection_tick) > NN_DETECTION_DELAY_TIME) &&
			(*(uint32_t *)detection_result != NN_DETECTION_RESET_STATE))
	{
		uint32_t det = 0;
		for (uint32_t i = 0; i < NN_DETECTION_COMMANDS_COUNT; i++)
		{
			if (command_data[i].label == *(uint32_t *)detection_result)
			{
				(*command_data[i].action)(i);
				det = 1;
				device.neuralNetworkId = 0;
				NN_ClearAudioBuffer();
			}
		}
		if (det) *(uint32_t *)detection_result = NN_DETECTION_RESET_STATE;
	}
}
