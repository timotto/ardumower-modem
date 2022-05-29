#pragma once

#include <list>
#include <functional>

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      class Attribute;
      class Attributes;
      class Measurement;

      extern std::list<Measurement *> _allMeasurements;

      class Attribute
      {
      public:
        const char *key;
        const char *value;
        Attribute() : key(nullptr), value(nullptr) {}
        Attribute(const char *_key, const char *_value) : key(_key), value(_value) {}

        int write(char *buf, int length);
        unsigned int length() const;
      };

      class Attributes
      {
      public:
        std::list<Attribute> attrs;
        Attributes() {}
        Attributes(Attribute a)
        {
          add(a);
        }

        Attributes &add(Attribute a);
        Attributes clone();
        int write(char *buf, int length);
        unsigned int length() const;
      };

      class Measurement
      {
      public:
        const char *_key;
        Attributes _attrs;
        Measurement(const char *key, Attributes attrs = Attributes());
        ~Measurement();

        int write(char *buf, int length);
        unsigned int length();

      protected:
        virtual unsigned int writeValue(char *buffer, unsigned int size) = 0;
        virtual unsigned int valueLength()
        {
          char buffer[64];
          return writeValue(buffer, sizeof(buffer));
        }
      };

      class CallbackMeasurement : public Measurement
      {
      private:
        std::function<unsigned int(char *, unsigned int)> _cb;

      public:
        CallbackMeasurement(const char *key, std::function<unsigned int(char *, unsigned int)> cb, Attributes attrs = Attributes())
            : Measurement(key, attrs), _cb(cb)
        {
        }

      protected:
        virtual unsigned int writeValue(char *buffer, unsigned int size) override
        {
          return _cb(buffer, size);
        }
      };
    }
  }
}
