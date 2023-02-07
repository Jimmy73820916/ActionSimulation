#pragma once

/*******************************************************************************
EasyVsp System
Copyright (c) 2022 Jimmy Song

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <optional>
#include <QString>
#include <string>
#include <vector>
#include <QJsonValue>
#include <QJsonArray>
#include <QColor>
#include "errorcode.h"

namespace Jimmy
{

namespace CommonFunction
{
double decPow(int32_t digits);

double normalizeDouble(double val, int32_t digits);
int64_t d2IEx(const double val, int32_t digits);

int32_t base64Encode(const uint8_t* binaryStream, size_t inSize, char** pOutString);
int32_t base64Decode(const char* base64String, uint8_t** binaryStream, size_t* outSize);

std::string lastErrorToString();

bool validEngName(const QString& cid);
bool validChsName(const QString& cid);

void splitCSVLine(const std::string& originalString, std::vector<std::string>& vSubStringGroup);

void checkJsonChar(char ch, bool& is_backslash, bool& is_quotation, int& num);

bool isAsciiString(const char* str);
bool isUtf8String(const char* str);

std::string UTF8toGBK(const std::string& utf8string);
std::wstring UTF8toUnicode(const std::string& utf8string);
std::string GBKtoUTF8(const std::string& gbkstring);
std::wstring GBKtoUnicode(const std::string& gbkstring);
std::string UnicodetoUTF8(const std::wstring& unicodestring);
std::string UnicodetoGBK(const std::wstring& unicodestring);

bool transform_jatosl(const QJsonArray arr,QStringList& sl);
std::optional<QStringList> transform_jatosl(const QJsonArray arr);

bool readJsonValue(const QJsonObject& jo,const QString& key, double* val);
bool readJsonValue(const QJsonObject& jo,const QString& key, QColor* val);

QJsonValue transferType(const QVariant& value);
QJsonValue transferType(const QString& value);
}

}


