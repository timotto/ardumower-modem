#include <Arduino.h>

namespace ArduMower
{
  namespace Util
  {
    class URL
    {
      private:
        String _scheme;
        String _hostname;
        int _port;
        String _path;
      public:
        URL(const String & _url);
        String scheme();
        String hostname();
        int port();
        String path();
    };
  }
}
