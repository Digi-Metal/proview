
#include <string.h>
#include <stdio.h>
 
extern "C" {
#include "co_dcli.h"
}
#include "co_lng.h"

lng_eLanguage Lng::lang = lng_eLanguage_en_us;

char Lng::items[][2][40] = {
      {"Help",         "Hj�lp"},
      {"Help Class",   "Hj�lp Klass"},
      {"Graph",        "Bild"},
      {"Trend",        "Trend"},
      {"Open Object",  "�ppna Objekt"},
      {"Open Object...", "�ppna Objekt"},
      {"RtNavigator",  "RtNavigat�r"},
      {"Class Graph",  "Klass Bild"},
      {"Crossreferences", "Korsreferenser"},
      {"DataSheet",    "Datablad"},
      {"Collect",      "Samla"},
      {"Type Graph...","Typ Bild..."}, 
      {"Open Plc...",  "�ppna Plc..."},
      {"Alarm list",   "Larm lista"},
      {"Event list",   "H�ndelse lista"},
      {"Link",         "L�nkar"},
      {"Subscription Client",  "Prenumerationer Klient"},
      {"Subscription Server",  "Prenumerationer Server"},
      {"Nethandler",   "N�thanterare"},
      {"Communication", "Kommunikation"},
      {"Device",       "Enheter"},
      {"PlcThread",    "PlcTr�dar"},
      {"PlcPgm",       "PlcPgm"},
      {"Logging",      "Loggning"},
      {"Logging entry 1", "Loggning entry 1"},
      {"Logging entry 2", "Loggning entry 2"},
      {"Logging entry 3", "Loggning entry 3"},
      {"Logging entry 4", "Loggning entry 4"},
      {"Logging entry 5", "Loggning entry 5"},
      {"Logging entry 6", "Loggning entry 6"},
      {"Logging entry 7", "Loggning entry 7"},
      {"Logging entry 8", "Loggning entry 8"},
      {"Logging entry 9", "Loggning entry 9"},
      {"Logging entry 10", "Loggning entry 10"},
      {"Database",     "Databas"},
      {"Alarm",        "Larm"},
      {"Store",        "Lagra"},
      {"System",       "System"},
      {"Exit",         "Avsluta"},
      {"Active",       "Aktiv"},
      {"Insert",       "L�ggIn"},
      {"Start",        "Starta"},
      {"Stop",         "Stoppa"},
      {"Restore",      "�terskapa"},
      {"ShowFile",     "VisaFil"},
      {"Time (ms)",    "Tid (ms)"},
      {"File",         "Fil"},
      {"Type",         "Typ"},
      {"BufferSize",   "BufferStorlek"},
      {"FullBufferStop","FullBufferStopp"},
      {"ShortName",    "KortNamn"},
      {"Parameter",    "Parameter"},
      {"ConditionParameter", "VillkorsParameter"},
      {"value >",      "v�rde >"},
      {"Description",  "Beskrivning"},
      {"InitialValue", "InitialV�rde"},
      {"ActualValue",  "NuV�rde"},
      {"Unit",         "Enhet"},
      {"Channel",      "Kanal"},
      {"Hold",         "H�ll"},
      {"Slider",       "Reglage"},
      {"FilterType",   "FilterTyp"},
      {"Value",        "V�rde"},
      {"Identity",     "Identitet"},
      {"Number",       "Nummer"},
      {"Range",        "Omr�de"},
      {"SensorSignalValue", "GivarSignalV�rde"},
      {"ChannelSignalValue", "KanalSignalV�rde"},
      {"RawValue",     "R�V�rde"},
      {"Update range", "Uppdatera omr�de"},
      {"Signal",       "Signal"},
      {"TestOn",       "TestTill"},
      {"TestValue",    "TestV�rde"},
      {"Mode",         "Mod"},
      {"Man",          "Man"},
      {"Auto",         "Auto"},
      {"Cascade",      "Kaskad"},
      {"Extern SetValue", "Externt B�rV�rde"},
      {"Force",        "Tvinga"},
      {"Set value",    "B�r v�rde"},
      {"Process value","�r v�rde"},
      {"Out value",    "Ut signal"},
      };


char *Lng::get_language_str()
{
  static char result[20];

  switch( lang) {
    case lng_eLanguage_sv_se:
      strcpy( result, "sv_se");
      break;
    default:
      strcpy( result, "en_us");
  }
  return result;
}

char *Lng::translate( char *str)
{
  static char result[200];
  char *in_p;

  if ( lang == lng_eLanguage_en_us) {
    // No translation is needed
    strcpy( result, str);
    return result;
  }

  // Remove space
  for ( in_p = str; *in_p == ' ' && *in_p; in_p++)
    ;

  for ( int i = 0; i < (int) (sizeof(items)/sizeof(items[0])); i++) {
    if ( strcmp( items[i][0], in_p) == 0) {
      strcpy( result, items[i][lang]);
      return result;
    }
  } 
  // Not found
  strcpy( result, str);
  return result;
}

int Lng::translate( char *in, char *out)
{
  char *in_p;

  if ( lang == lng_eLanguage_en_us) {
    // No translation is needed
    return 0;
  }

  // Remove space
  for ( in_p = in; *in_p == ' ' && *in_p; in_p++)
    ;

  for ( int i = 0; i < (int) (sizeof(items)/sizeof(items[0])); i++) {
    if ( strcmp( items[i][0], in_p) == 0) {
      strcpy( out, items[i][lang]);
      return 1;
    }
  } 
  // Not found
  return 0;
}

void Lng::get_uid( char *in, char *out)
{
  char result[200];

#if defined OS_VMS
  {
    char dev[80], dir[80], file[80], type[80];
    int version;
    char c;

    dcli_parse_filename( "pwr_exe:", dev, dir, file, type, &version);
    sprintf( result, "%s%s", dev, dir);
    c = result[strlen(result)-1];
    sprintf( &result[strlen(result)-1], ".%s%c%s%", get_language_str(), 
	     c, in);
  }
#else
  sprintf( result, "$pwr_exe/%s/%s", get_language_str(), in);
  dcli_translate_filename( result, result);
#endif
  strcpy( out, result);
}












