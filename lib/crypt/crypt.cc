// crypt.cc
// 9/4/2011

#include "lib/crypt/crypt.h"
#include "lib/crypt/simplecrypt.h"
#include <QtCore/QCryptographicHash>

QByteArray
Crypt::sha256(const QByteArray &input)
{
  // TODO: need openssl and -lcrypto
  // http://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
  Q_UNUSED(input)
  return QByteArray();
}

QByteArray
Crypt::sha1(const QByteArray &input)
{
  QCryptographicHash digest(QCryptographicHash::Sha1);
  digest.addData(input);
  return digest.result();
}

QByteArray
Crypt::md5(const QByteArray &input)
{
  QCryptographicHash digest(QCryptographicHash::Md5);
  digest.addData(input);
  return digest.result();
}

QByteArray
Crypt::hex(const QByteArray &input)
{ return input.toHex(); }

QByteArray
Crypt::encrypt(const QByteArray &input, quint64 key)
{ return SimpleCrypt(key).encryptToByteArray(input); }

QByteArray
Crypt::decrypt(const QByteArray &input, quint64 key)
{ return SimpleCrypt(key).decryptToByteArray(input); }

QString
Crypt::encrypt(const QString &input, quint64 key)
{ return SimpleCrypt(key).encryptToString(input); }

QString
Crypt::decrypt(const QString &input, quint64 key)
{ return SimpleCrypt(key).decryptToString(input); }

// EOF
