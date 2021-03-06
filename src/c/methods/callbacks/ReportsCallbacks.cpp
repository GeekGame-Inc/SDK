#include "c/methods/callbacks/ReportsCallbacks.h"

std::map<u32, GenericRequestParams *> submit_report_callbacks;

void modioOnSubmitReport(u32 call_number, u32 response_code, nlohmann::json response_json)
{
  ModioResponse response;
  modioInitResponse(&response, response_json);
  response.code = response_code;

  submit_report_callbacks[call_number]->callback(submit_report_callbacks[call_number]->object, response);

  delete submit_report_callbacks[call_number];
  submit_report_callbacks.erase(call_number);

  modioFreeResponse(&response);
}

void clearReportsCallbackParams()
{
  for (auto submit_report_callback : submit_report_callbacks)
    delete submit_report_callback.second;
  submit_report_callbacks.clear();
}
