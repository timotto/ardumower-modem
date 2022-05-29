#include <AUnit.h>
using namespace aunit;

#include "../src/encrypt.h"

class EncryptionTestFixture
{
public:
  const char *plaintext;
  const char *ciphertext;
  const int password;
  const int challenge;
  EncryptionTestFixture(const char *plaintext, const char *ciphertext, const int password, const int challenge);
};

class TestEncrypt : public TestOnce
{
private:
protected:
  ArduMower::Encrypt uut;
  char *buffer;

  void setup() override;
  void teardown() override;

  void withEachFixture(std::function<void(EncryptionTestFixture)> fn);
  void apply(EncryptionTestFixture &f);
  char *encrypt(const char *plaintext);
  char *decrypt(const char *ciphertext);
};

testF(TestEncrypt, decryption)
{
  withEachFixture(
      [&](EncryptionTestFixture f)
      {
        apply(f);
        const char *actualResult = decrypt(f.ciphertext);
        assertEqual(actualResult, f.plaintext);
      });
}

testF(TestEncrypt, encryption)
{
  withEachFixture(
      [&](EncryptionTestFixture f)
      {
        apply(f);
        const char *actualResult = encrypt(f.plaintext);
        assertEqual(actualResult, f.ciphertext);
      });
}

test(encrypt_password_effect)
{
  const char *input = "some input";

  ArduMower::Encrypt uut;
  uut.setOn(true);
  uut.setChallenge(143);

  char *buffer1, *buffer2;
  buffer1 = strdup(input);
  buffer2 = strdup(input);

  uut.setPassword(123456);
  uut.encrypt(buffer1, strlen(buffer1));

  uut.setPassword(923456);
  uut.encrypt(buffer2, strlen(buffer2));

  assertNotEqual(buffer1, buffer2);

  free(buffer1);
  free(buffer2);
}

test(encrypt_challenge_effect)
{
  const char *input = "some input";
  char *buffer1, *buffer2;
  buffer1 = strdup(input);
  buffer2 = strdup(input);

  ArduMower::Encrypt uut;
  uut.setOn(true);
  uut.setPassword(123456);

  uut.setChallenge(143);
  uut.encrypt(buffer1, strlen(buffer1));

  uut.setChallenge(7);
  uut.encrypt(buffer2, strlen(buffer2));

  assertNotEqual(buffer1, buffer2);

  free(buffer1);
  free(buffer2);
}

// testbed

auto encryptionTestFixture1 = EncryptionTestFixture("AT+C,-1,1,0.1,100,0,-1,-1,1,0x2f",
                                                    "p$Zr[\\`[`[_]`[`__[_[\\`[\\`[`[_Ha6",
                                                    123456, 143);
auto encryptionTestFixture2 = EncryptionTestFixture("AT+C,-1,0,-1,-1,-1,-1,-1,-1,0x25",
                                                    "p$Zr[\\`[_[\\`[\\`[\\`[\\`[\\`[\\`[_Had",
                                                    123456, 143);
auto encryptionTestFixtures = {encryptionTestFixture1, encryptionTestFixture2};

EncryptionTestFixture::EncryptionTestFixture(const char *pt, const char *ct, const int pw, const int ch)
    : plaintext(pt), ciphertext(ct), password(pw), challenge(ch) {}

void TestEncrypt::setup()
{
  buffer = nullptr;
}

void TestEncrypt::teardown()
{
  if (buffer != nullptr)
    free(buffer);
  buffer = nullptr;
}

void TestEncrypt::apply(EncryptionTestFixture &f)
{
  uut.setOn(true);
  uut.setPassword(f.password);
  uut.setChallenge(f.challenge);
}

char *TestEncrypt::encrypt(const char *plaintext)
{
  if (buffer != nullptr)
    free(buffer);
  buffer = strdup(plaintext);
  uut.encrypt(buffer, strlen(buffer));
  return buffer;
}

char *TestEncrypt::decrypt(const char *ciphertext)
{
  if (buffer != nullptr)
    free(buffer);
  buffer = strdup(ciphertext);
  uut.decrypt(buffer, strlen(buffer));
  return buffer;
}

void TestEncrypt::withEachFixture(std::function<void(EncryptionTestFixture)> fn)
{
  for (auto fixture : encryptionTestFixtures)
    fn(fixture);
}
