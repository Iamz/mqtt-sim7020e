#include <pgmspace.h>

#define SECRET
#define THINGNAME "YOUR THING NAME"

const char AWS_IOT_ENDPOINT[] = "YOUR ENDPOINT";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----\n<CERTIFICATE BODY>\n-----END CERTIFICATE-----)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(-----BEGIN CERTIFICATE-----\n<CERTIFICATE BODY>\n-----END CERTIFICATE-----)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(-----BEGIN RSA PRIVATE KEY-----\n<PRIVATE KEY BODY>\n-----END RSA PRIVATE KEY-----)KEY";
