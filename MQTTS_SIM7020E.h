#include <HardwareSerial.h>

class MQTTS_SIM7020E {
  public:
    MQTTS_SIM7020E();
    void setupModule();
    void rebootModule();
    void checkModuleReady();
    void waitForNetwork();
    String dnsLookup(String hostname);
    void setCertificate(int type, String cert);
    void checkCertificates();
    void connect(String ipAddress, String port);
    void registerClient(String clientName);
    void publish(String topic, String payload);
    void disconnect();
    bool debug;
    HardwareSerial _serial;
  private:
    void _clearCertificate(int type);
    void _flushSerial();
    String _data_input;
};
