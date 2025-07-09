#include "test.hh"
#include <cstdio>
#include <cuda.h>
#include <string>

namespace GoPTX {
namespace test {
int now_cnt = 0;
}
} // namespace GoPTX

int main() {
  CUdevice device;
  CUcontext context;
  DRIVER_API_CALL(cuInit(0));
  DRIVER_API_CALL(cuDeviceGet(&device, 0));
  DRIVER_API_CALL(cuDevicePrimaryCtxRetain(&context, device));
  DRIVER_API_CALL(cuCtxPushCurrent(context));

  std::string json = GoPTX::test::test();

  DRIVER_API_CALL(cuCtxPopCurrent(&context));
  DRIVER_API_CALL(cuDevicePrimaryCtxRelease(device));

  std::string html =
      "<!DOCTYPE html>\n<html>\n\n<head>\n <script>\n  var "
      "dataset_GoPTX_test = " +
      json +
      ";\n </script>\n <style>\n  .error {\n   color: red;\n  }\n </style>\n "
      "<script type=\"text/javascript\" "
      "src=\"https://cdn.jsdelivr.net/npm//vega@5\"></script>\n <script "
      "type=\"text/javascript\" "
      "src=\"https://cdn.jsdelivr.net/npm//vega-lite@4.17.0\"></script>\n "
      "<script type=\"text/javascript\" "
      "src=\"https://cdn.jsdelivr.net/npm//vega-embed@6\"></script>\n</"
      "head>\n\n<body>\n <div id=\"vis\"></div>\n <script>\n  (function "
      "(vegaEmbed) {\n   var spec = { \"data\": { \"name\": "
      "\"dataset_GoPTX_test\" }, \"mark\": \"bar\", \"encoding\": { "
      "\"color\": { \"field\": \"Strategy\", \"type\": \"nominal\" }, "
      "\"column\": { \"field\": \"Workload\", \"type\": \"nominal\" }, \"x\": "
      "{ \"field\": "
      "\"Strategy\", \"type\": \"nominal\" }, \"y\": { \"field\": "
      "\"min\\\\(ms\\\\)\", \"type\": \"quantitative\" } }, \"$schema\": "
      "\"https://vega.github.io/schema/vega-lite/v4.17.0.json\", \"datasets\": "
      "{ \"dataset_GoPTX_test\": dataset_GoPTX_test } };\n   var "
      "embedOpt "
      "= { \"mode\": \"vega-lite\" };\n\n   function showError(el, error) {\n  "
      "  el.innerHTML = (\'<div class=\"error\" style=\"color:red;\">\'\n     "
      "+ \'<p>JavaScript Error: \' + error.message + \'</p>\'\n     + "
      "\"<p>This usually means there\'s a typo in your chart specification. "
      "\"\n     + \"See the javascript console for the full traceback.</p>\"\n "
      "    + \'</div>\');\n    throw error;\n   }\n   const el = "
      "document.getElementById(\'vis\');\n   vegaEmbed(\"#vis\", spec, "
      "embedOpt)\n    .catch(error => showError(el, error));\n  "
      "})(vegaEmbed);\n\n </script>\n</body>\n\n</html>\n";
  std::fprintf(stdout, "%s", html.c_str());
}