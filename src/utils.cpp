#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <cctype>

// To test the to_json() function, uncomment the following
// line and compile with gcc:
// #define TEST_TO_JSON 1
// #define TEST_TRACE   1

// If you want the to_json() function to keep spaces in the output,
// uncomment the following line
// #define KEEP_SPACES 1

// ------

#ifndef TEST_TO_JSON

#include <esp_log.h>

#include "config.hpp"

void dump_data(const char *tag, const uint8_t * data, int len) 
{
  esp_log_level_t log_level = esp_log_level_get(tag);

  if ((log_level != ESP_LOG_DEBUG) && (log_level != ESP_LOG_VERBOSE)) return;

  int pos = 0;
  static char buff[80];

  while (pos < len) {
    sprintf(buff, "%03XH:", pos);
    int k = 5;
    for (int i = 0; i < 16; i++) {
      if ((pos + i) < len) {
        sprintf(&buff[k], " %02X", data[pos + i]);
      }
      else {
        strcpy(&buff[k], "   ");
      }
      k += 3;
    }
    strcpy(&buff[k], "  |");
    k += 3;
    for (int i = 0; i < 16; i++) {
      buff[k++] = ((pos + i) < len) 
                    ? (((data[pos + i] >= ' ') && (data[pos + i] <= '~')) 
                        ? data[pos + i] 
                        : '.') 
                    : ' ';
    }
    strcpy(&buff[k], "|");
    ESP_LOGD(tag, "%s", buff);

    pos += 16;
  }
}

#endif

#if defined(TEST_TO_JSON) && defined(TEST_TRACE)
  #define SHOW(n) printf("     %s\n", n);
  #define SHOWC(c) putchar('['); putchar(c); putchar(']');
#else
  #define SHOW(n)
  #define SHOWC(c)
#endif

#define GET(c) if (ilen > 0) { c = *ibuff++; ilen--; } else { c = 0; }
#define PUT(c) if (olen > 0) { *obuff++ = c; olen--; SHOWC(c); } else { return false; }

#ifdef KEEP_SPACES
  #define SKIP_SPACES while (isspace(ch)) { PUT(ch); GET(ch); }
#else
  #define SKIP_SPACES while (isspace(ch)) { GET(ch); }
#endif

static char * obuff;
static char * ibuff;
static int    ilen;
static int    olen; 
static char   ch;

static bool do_item();

static bool do_object()
{
  SHOW("do_object()");

  if (ch != '{') return false;

  PUT(ch); GET(ch); SKIP_SPACES;

  if ((ch != '}') && (ch != ',')) {
    do {
      if (ch == ',') { PUT(ch); GET(ch); SKIP_SPACES; }
      if (!do_item()) return false;
    } while (ch == ',');
  }

  if (ch != '}') return false;
  PUT(ch); GET(ch); SKIP_SPACES;

  return true;
}

static bool do_string()
{
  SHOW("do_string()");

  if (ch == '"') {
    PUT(ch); GET(ch);
    while (ch != '"') {
      if (ch == 0) return false;
      if (ch == '\\') {
        PUT(ch); GET(ch); PUT(ch);
      }
      else {
        PUT(ch); 
      }
      GET(ch);
    }
    PUT(ch); GET(ch);
  }
  else {
    PUT('"');
    while (!((ch == 0) || isspace(ch) || (ch == ':') || (ch == ',') || (ch == '}') || (ch == ']'))) {
      PUT(ch); GET(ch);
    }
    PUT('"');
  }

  SKIP_SPACES;

  return true;
}

static bool do_array();

static bool do_value()
{
  SHOW("do_value()");
  
  if ((ilen > 5) && (strncmp("false", ibuff-1, 5) == 0)) {
    for (int i = 0; i < 5; i++) { PUT(ch); GET(ch); }
    SKIP_SPACES;
  }
  else if ((ilen > 4) && (strncmp("true", ibuff-1, 4) == 0)) {
    for (int i = 0; i < 4; i++) { PUT(ch); GET(ch); }
    SKIP_SPACES;
  }
  else if ((ilen > 4) && (strncmp("null", ibuff-1, 4) == 0)) {
    for (int i = 0; i < 4; i++) { PUT(ch); GET(ch); }
    SKIP_SPACES;
  }
  else if (isdigit(ch) || (ch == '-')) {
    PUT(ch); GET(ch);
    while (isdigit(ch) || (ch == '.')) { PUT(ch); GET(ch); }
    if ((ch == 'E') || (ch == 'e')) {
      PUT(ch); GET(ch);
      if ((ch == '+' || (ch == '-'))) { PUT(ch); GET(ch); }
      while (isdigit(ch)) { PUT(ch); GET(ch); }
    }
    SKIP_SPACES;
  }
  else if (ch == '{') {
    return do_object();
  }
  else if (ch == '[') {
    return do_array();
  }
  else {
    return do_string();
  }

  return true;
}

bool do_array()
{
  SHOW("do_array()");
  
  if (ch != '[') return false;
  PUT(ch); GET(ch); SKIP_SPACES;

  while (ch != ']') {
    if (ch == 0) return false;
    do_value();
    if (ch == ',') { 
      PUT(ch); GET(ch); SKIP_SPACES;
    }
    else break; 
  }
  
  if (ch != ']') return false;

  PUT(ch); GET(ch); SKIP_SPACES;
  return true;
}

static bool do_item()
{
  SHOW("do_item()");
  
  if (!do_string()) return false;

  if (ch != ':') return false;
  PUT(ch); GET(ch); SKIP_SPACES;

  return do_value();
}

int to_json(uint8_t * in_buffer, int in_len, char * out_buffer, int out_len)
{
  SHOW("to_json()");

  if (out_len < in_len) return -1;

  obuff = out_buffer;
  ibuff = (char *) in_buffer;
  olen  = out_len;
  ilen  = in_len;

  GET(ch); SKIP_SPACES;

  if (!do_object()) return -1;
  while (ch != 0) { PUT(ch); GET(ch); }
  if (olen > 0) *obuff = 0;

  return out_len - olen;
}

// ----------------------------

#ifdef TEST_TO_JSON

int err_count = 0;

char out[10000];

const char * test1     = "{\"string1\":\"string2\"}";
const char * test1_res = "{\"string1\":\"string2\"}";

const char * test2     = "{string1:\"string2\"}";
const char * test2_res = "{\"string1\":\"string2\"}";

const char * test3     = "{string1:string2}";
const char * test3_res = "{\"string1\":\"string2\"}";

const char * test4     = "{string1:1234}";
const char * test4_res = "{\"string1\":1234}";

const char * test5     = "{string1:[-1234e-10,true,{aa:bb},false,null,boo]} rest";
const char * test5_res = "{\"string1\":[-1234e-10,true,{\"aa\":\"bb\"},false,null,\"boo\"]} rest";

const char * test6     = "{  }";
const char * test6_res = "{  }";

const char * test7     = "{ empty: [] }";
const char * test7_res = "{ \"empty\": [] }";

const char * test8     = R"st(
{widget: {
    debug: on,
    window: {
        title: "Sample Konfabulator Widget",
        name: main_window,
        width: 500,
        height: 500
    },
    image: { 
        src: Images/Sun.png,
        name: sun1,
        hOffset: 250,
        vOffset: 250,
        alignment: center
    },
    text: {
        data: "Click Here",
        size: 36,
        style: bold,
        name: text1,
        hOffset: 250,
        vOffset: 100,
        alignment: center,
        onMouseUp: "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
}}    
)st";

const char * test8_res     = R"st(
{"widget": {
    "debug": "on",
    "window": {
        "title": "Sample Konfabulator Widget",
        "name": "main_window",
        "width": 500,
        "height": 500
    },
    "image": { 
        "src": "Images/Sun.png",
        "name": "sun1",
        "hOffset": 250,
        "vOffset": 250,
        "alignment": "center"
    },
    "text": {
        "data": "Click Here",
        "size": 36,
        "style": "bold",
        "name": "text1",
        "hOffset": 250,
        "vOffset": 100,
        "alignment": "center",
        "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
}}    
)st";

const char * test9 = R"st(
{web-app: {
  servlet: [   
    {
      servlet-name: cofaxCDS,
      servlet-class: org.cofax.cds.CDSServlet,
      init-param: {
        "configGlossary:installationAt": "Philadelphia, PA",
        "configGlossary:adminEmail": ksm@pobox.com,
        "configGlossary:poweredBy": Cofax,
        "configGlossary:poweredByIcon": /images/cofax.gif,
        "configGlossary:staticPath": /content/static,
        "templateProcessorClass": org.cofax.WysiwygTemplate,
        templateLoaderClass: org.cofax.FilesTemplateLoader,
        templatePath: templates,
        templateOverridePath: "",
        defaultListTemplate: listTemplate.htm,
        defaultFileTemplate: articleTemplate.htm,
        useJSP: false,
        jspListTemplate: listTemplate.jsp,
        jspFileTemplate: articleTemplate.jsp,
        cachePackageTagsTrack: 200,
        cachePackageTagsStore: 200,
        cachePackageTagsRefresh: 60,
        cacheTemplatesTrack: 100,
        cacheTemplatesStore: 50,
        cacheTemplatesRefresh: 15,
        cachePagesTrack: 200,
        cachePagesStore: 100,
        cachePagesRefresh: 10,
        cachePagesDirtyRead: 10,
        searchEngineListTemplate: forSearchEnginesList.htm,
        searchEngineFileTemplate: forSearchEngines.htm,
        searchEngineRobotsDb: WEB-INF/robots.db,
        useDataStore: true,
        dataStoreClass: org.cofax.SqlDataStore,
        redirectionClass: org.cofax.SqlRedirection,
        dataStoreName: cofax,
        dataStoreDriver: com.microsoft.jdbc.sqlserver.SQLServerDriver,
        dataStoreUrl: "jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon",
        dataStoreUser: sa,
        dataStorePassword: dataStoreTestQuery,
        dataStoreTestQuery: "SET NOCOUNT ON;select test='test';",
        dataStoreLogFile: /usr/local/tomcat/logs/datastore.log,
        dataStoreInitConns: 10,
        dataStoreMaxConns: 100,
        dataStoreConnUsageLimit: 100,
        dataStoreLogLevel: debug,
        maxUrlLength: 500}},
    {
      servlet-name: cofaxEmail,
      servlet-class: org.cofax.cds.EmailServlet,
      init-param: {
      mailHost: mail1,
      mailHostOverride: mail2}},
    {
      servlet-name: cofaxAdmin,
      servlet-class: org.cofax.cds.AdminServlet},
 
    {
      servlet-name: fileServlet,
      servlet-class: org.cofax.cds.FileServlet},
    {
      servlet-name: cofaxTools,
      servlet-class: org.cofax.cms.CofaxToolsServlet,
      init-param: {
        templatePath: toolstemplates/,
        log: 1,
        logLocation: /usr/local/tomcat/logs/CofaxTools.log,
        logMaxSize: "",
        dataLog: 1,
        dataLogLocation: /usr/local/tomcat/logs/dataLog.log,
        dataLogMaxSize: "",
        removePageCache: /content/admin/remove?cache=pages&id=,
        removeTemplateCache: /content/admin/remove?cache=templates&id=,
        fileTransferFolder: /usr/local/tomcat/webapps/content/fileTransferFolder,
        lookInContext: 1,
        adminGroupID: 4,
        betaServer: true}}],
  servlet-mapping: {
    cofaxCDS: /,
    cofaxEmail: /cofaxutil/aemail/*,
    cofaxAdmin: /admin/*,
    fileServlet: /static/*,
    cofaxTools: /tools/*},
 
  taglib: {
    taglib-uri: cofax.tld,
    taglib-location: /WEB-INF/tlds/cofax.tld}}}
)st";

const char * test9_res = R"st(
{"web-app": {
  "servlet": [   
    {
      "servlet-name": "cofaxCDS",
      "servlet-class": "org.cofax.cds.CDSServlet",
      "init-param": {
        "configGlossary:installationAt": "Philadelphia, PA",
        "configGlossary:adminEmail": "ksm@pobox.com",
        "configGlossary:poweredBy": "Cofax",
        "configGlossary:poweredByIcon": "/images/cofax.gif",
        "configGlossary:staticPath": "/content/static",
        "templateProcessorClass": "org.cofax.WysiwygTemplate",
        "templateLoaderClass": "org.cofax.FilesTemplateLoader",
        "templatePath": "templates",
        "templateOverridePath": "",
        "defaultListTemplate": "listTemplate.htm",
        "defaultFileTemplate": "articleTemplate.htm",
        "useJSP": false,
        "jspListTemplate": "listTemplate.jsp",
        "jspFileTemplate": "articleTemplate.jsp",
        "cachePackageTagsTrack": 200,
        "cachePackageTagsStore": 200,
        "cachePackageTagsRefresh": 60,
        "cacheTemplatesTrack": 100,
        "cacheTemplatesStore": 50,
        "cacheTemplatesRefresh": 15,
        "cachePagesTrack": 200,
        "cachePagesStore": 100,
        "cachePagesRefresh": 10,
        "cachePagesDirtyRead": 10,
        "searchEngineListTemplate": "forSearchEnginesList.htm",
        "searchEngineFileTemplate": "forSearchEngines.htm",
        "searchEngineRobotsDb": "WEB-INF/robots.db",
        "useDataStore": true,
        "dataStoreClass": "org.cofax.SqlDataStore",
        "redirectionClass": "org.cofax.SqlRedirection",
        "dataStoreName": "cofax",
        "dataStoreDriver": "com.microsoft.jdbc.sqlserver.SQLServerDriver",
        "dataStoreUrl": "jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon",
        "dataStoreUser": "sa",
        "dataStorePassword": "dataStoreTestQuery",
        "dataStoreTestQuery": "SET NOCOUNT ON;select test='test';",
        "dataStoreLogFile": "/usr/local/tomcat/logs/datastore.log",
        "dataStoreInitConns": 10,
        "dataStoreMaxConns": 100,
        "dataStoreConnUsageLimit": 100,
        "dataStoreLogLevel": "debug",
        "maxUrlLength": 500}},
    {
      "servlet-name": "cofaxEmail",
      "servlet-class": "org.cofax.cds.EmailServlet",
      "init-param": {
      "mailHost": "mail1",
      "mailHostOverride": "mail2"}},
    {
      "servlet-name": "cofaxAdmin",
      "servlet-class": "org.cofax.cds.AdminServlet"},
 
    {
      "servlet-name": "fileServlet",
      "servlet-class": "org.cofax.cds.FileServlet"},
    {
      "servlet-name": "cofaxTools",
      "servlet-class": "org.cofax.cms.CofaxToolsServlet",
      "init-param": {
        "templatePath": "toolstemplates/",
        "log": 1,
        "logLocation": "/usr/local/tomcat/logs/CofaxTools.log",
        "logMaxSize": "",
        "dataLog": 1,
        "dataLogLocation": "/usr/local/tomcat/logs/dataLog.log",
        "dataLogMaxSize": "",
        "removePageCache": "/content/admin/remove?cache=pages&id=",
        "removeTemplateCache": "/content/admin/remove?cache=templates&id=",
        "fileTransferFolder": "/usr/local/tomcat/webapps/content/fileTransferFolder",
        "lookInContext": 1,
        "adminGroupID": 4,
        "betaServer": true}}],
  "servlet-mapping": {
    "cofaxCDS": "/",
    "cofaxEmail": "/cofaxutil/aemail/*",
    "cofaxAdmin": "/admin/*",
    "fileServlet": "/static/*",
    "cofaxTools": "/tools/*"},
 
  "taglib": {
    "taglib-uri": "cofax.tld",
    "taglib-location": "/WEB-INF/tlds/cofax.tld"}}}
)st";

const char * test10     = "  {  string1  :  [  -1234e-10  ,  true  ,  {  aa  :  bb  }  ,  false  ,  null  ,  boo  ]  }  rest";
const char * test10_res = "  {  \"string1\"  :  [  -1234e-10  ,  true  ,  {  \"aa\"  :  \"bb\"  }  ,  false  ,  null  ,  \"boo\"  ]  }  rest";

int check(const char * a, const char * b)
{
  int idx = 0;

  while ((*a != 0) && (*b != 0)) {
    if (*a++ != *b++) break;
    idx ++;
  }
  if ((*a == 0) && (*b == 0)) return -1;
  return idx;
}

void testing(const char * name, const char * json, const char * ans)
{
  int idx;

  memset(out, 0, sizeof(out));

  //printf("Testing %s...\n", json);
  int size = to_json((uint8_t *) json, strlen(json), out, sizeof(out));
  if (size == -1) {
    printf("Test %s error at %d!!: %*s\n", name, (int)(strlen(json) - ilen), (int)(sizeof(out) - olen), out);
    err_count++;
  }
  else if ((idx = check(out, ans)) == -1) {
    printf("Test %-6s OK.\n", name);
  }
  else {
    printf("Test %s output different at %d:\n-- expect --\n%s\n-- result --\n%s\n---------\n", name, idx, ans, out);
    err_count++;
  }
}

int main()
{
  printf("JSON Testing...\n\n");

  testing("test1",  test1,  test1_res );
  testing("test2",  test2,  test2_res );
  testing("test3",  test3,  test3_res );
  testing("test4",  test4,  test4_res );
  testing("test5",  test5,  test5_res );
  testing("test6",  test6,  test6_res );
  testing("test7",  test7,  test7_res );
  testing("test8",  test8,  test8_res );
  testing("test9",  test9,  test9_res );
  testing("test10", test10, test10_res);

  printf("\nEnd of tests, error count = %d\n", err_count);
}

#endif
