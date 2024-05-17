#include <SimpleHOTP.h> // for sha1
struct OpenMeteoLocation {
  String name;
  String latitude;
  String longitude;
  String timezone;
  String country_code;
};

#define BUFFER_SIZE 20000 // Adjust the size as needed
uint8_t meteolocation_buffer[BUFFER_SIZE];

String meteolocation_filename;
String om_payload = "";


String getLocationFilename(String token){
  String prefix = "/";
  String suffix = "";
  token.replace(" ","_");
  String filename = prefix+token+suffix;
  filename.toLowerCase();
  return filename;
}

void setupopenmeteo(){
    SPIFFS.begin(true); // Initialize the SPIFFS filesystem
}

bool saveProxyFile(String data, String token, int ttl_seconds){
  // Write the byte array to a file
  meteolocation_filename=getLocationFilename(token);
  File file = SPIFFS.open(meteolocation_filename+".ts", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  int expires = (millis()/1000)+ttl_seconds;
  file.write((uint8_t*)&expires, sizeof(expires));
  file.close();

  file = SPIFFS.open(meteolocation_filename, "w");
  Serial.println(meteolocation_filename);
  if (file) {
    file.write((uint8_t *) data.c_str(), data.length());
    file.close();
    Serial.println("Location saved to file.");
    return true;
  } else {
    Serial.println("Failed to open file for writing.");
  }
  return false;
}

String loadProxyFile(String token, int ttl_seconds){
  // Read the byte array from the file and deserialize it
  meteolocation_filename = getLocationFilename(token);
  File readFile = SPIFFS.open(meteolocation_filename+".ts", "r");
  if (readFile.size() > 0) {
    int expires = 0;
    readFile.readBytes((char*)&expires, sizeof(expires));
    readFile.close();
    if (((millis()/1000) > expires) //expired?
        || (abs(int(expires - (millis()/1000)) > ttl_seconds))) //clock overflow?
    {
      return "";
    }
  }
  else{
    return "";
  }
  readFile = SPIFFS.open(meteolocation_filename, "r");
  Serial.println(meteolocation_filename);
  if (readFile.size() > 0) {
    size_t fileSize = readFile.size();
    Serial.println(fileSize);
    readFile.read(meteolocation_buffer, fileSize);
    readFile.close();
    return String((char *) meteolocation_buffer);
  } else {
    Serial.println("Failed to open file for reading.");
  }
  return "";
}


String GetUrlProxyToken(const char * data, int size) {  
    uint32_t hash[5] = {}; // This will contain the 160-bit Hash
    SimpleSHA1::generateSHA((uint8_t *) data, size * 8, hash);
    
    String hashStr((const char*)nullptr);
    hashStr.reserve(20 * 2 + 1);

    for(uint8_t i = 0; i < 5; i++) {
        char hex[8];
        snprintf(hex, sizeof(hex), "%08x", hash[i]);
        hashStr += hex;
    }
    String hashRet = "";
    int c=0;
    for (int i=0; i<hashStr.length(); i+=2){
      hashRet += hashStr[i];
      c++;
    }
    return hashRet;
}

String ProxyGetRequest(String endpoint, long int ttl_seconds){
  String proxyhash = GetUrlProxyToken(endpoint.c_str(), endpoint.length());
  Serial.println(endpoint);
  Serial.println(proxyhash);
  om_payload = loadProxyFile(proxyhash, ttl_seconds);
  if (om_payload.length() > 0){
    return om_payload;
  }
  Serial.println(endpoint);  
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(endpoint.c_str()); // Specify the URL
    int httpCode = http.GET(); // Make the request

    if (httpCode > 0) { // Check for the returning code
      om_payload = http.getString();
      saveProxyFile(om_payload, proxyhash, ttl_seconds);
      //Serial.println(payload);
    } else {
      Serial.println("Error on HTTP request");
      Serial.println(httpCode);
    }

    http.end(); // Free the resources
  }
  return om_payload;
}
