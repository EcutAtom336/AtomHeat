#include "heat_info.h"

static void heat_stage_keep_vaild(HeatStage_t* p_heat_stage) {
  p_heat_stage->temp > 320 ? p_heat_stage->temp = 320 : 0;
  p_heat_stage->temp < 100 ? p_heat_stage->temp = 100 : 0;
  p_heat_stage->duartion_s > 1200 && p_heat_stage->duartion_s < 32768
      ? p_heat_stage->duartion_s = 1200
      : 0;
  p_heat_stage->duartion_s > 32768 ? p_heat_stage->duartion_s = 0 : 0;
}

void heat_info_revise(HeatInfo_t* p_heat_info) {
  p_heat_info->power_limit > 100 ? p_heat_info->power_limit = 100 : 0;
  p_heat_info->power_limit < 60 ? p_heat_info->power_limit = 60 : 0;

  heat_stage_keep_vaild(&p_heat_info->stage[0]);
  heat_stage_keep_vaild(&p_heat_info->stage[1]);
  heat_stage_keep_vaild(&p_heat_info->stage[2]);
  heat_stage_keep_vaild(&p_heat_info->stage[3]);
  heat_stage_keep_vaild(&p_heat_info->stage[4]);
}