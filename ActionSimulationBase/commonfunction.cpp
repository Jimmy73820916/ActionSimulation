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

#include "commonconst.h"
#include "commonfunction.h"
#include <QCoreApplication>
#include <boost/tokenizer.hpp>
#include <mutex>
#include "gadget.h"
#include <windows.h>
#include <assert.h>
#include <regex>
#include <locale>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

using namespace std;
using namespace boost;

namespace Jimmy
{
namespace CommonFunction
{

std::unordered_map<char, uint8_t> mBase64CharIndex_;
std::once_flag Base64CharIndexInited;

static const char* pBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void InitBase64CharIndex()
{
    for (uint8_t iPos = 0; iPos < static_cast<uint8_t>(strlen(pBase64Chars)); iPos++)
    {
        mBase64CharIndex_.insert(std::make_pair(pBase64Chars[iPos], iPos));
    }
}

inline bool Isbase64(char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

int32_t base64Encode(const uint8_t* binaryStream, size_t inSize, char** pOutString)
{
    std::call_once(Base64CharIndexInited, &InitBase64CharIndex);

    if (inSize == 0)
    {
        return ErrorCode::ec_invalid_param;
    }

    size_t bufferSize = inSize / 3 * 4 + 1;
    auto modul = inSize % 3;
    if (modul != 0)
    {
        bufferSize += 4;
    }

    *pOutString = new char[bufferSize];

    size_t inPos(0), outPos(0);
    char current('\0');
    for (; inPos < inSize; inPos += 3)
    {
        current = (binaryStream[inPos] >> 2);
        current &= 0x3F;
        (*pOutString)[outPos++] = pBase64Chars[static_cast<int>(current)];

        current = (binaryStream[inPos] << 4) & 0x30;
        if (inPos + 1 >= inSize)
        {
            (*pOutString)[outPos++] = pBase64Chars[static_cast<int>(current)];
            (*pOutString)[outPos++] = '=';
            (*pOutString)[outPos++] = '=';
            break;
        }

        current |= ((binaryStream[inPos + 1] >> 4) & 0x0F);
        (*pOutString)[outPos++] = pBase64Chars[static_cast<int>(current)];

        current = (binaryStream[inPos + 1] << 2) & 0x3C;
        if (inPos + 2 >= inSize)
        {
            (*pOutString)[outPos++] = pBase64Chars[(int)current];
            (*pOutString)[outPos++] = '=';
            break;
        }
        current |= ((binaryStream[inPos + 2] >> 6) & 0x03);
        (*pOutString)[outPos++] = pBase64Chars[static_cast<int>(current)];

        current = (binaryStream[inPos + 2] & 0x3F);
        (*pOutString)[outPos++] = pBase64Chars[static_cast<int>(current)];
    }

    (*pOutString)[outPos] = '\0';
    return ErrorCode::ec_ok;
}

int32_t base64Decode(const char* base64String, uint8_t** binaryStream, size_t* outSize)
{
    std::call_once(Base64CharIndexInited, &InitBase64CharIndex);

    if (base64String == nullptr)
    {
        return ErrorCode::ec_invalid_param;
    }

    size_t inSize = strlen(base64String);
    if (inSize == 0)
    {
        return ErrorCode::ec_invalid_param;
    }

    *outSize = inSize / 4 * 3;
    (*binaryStream) = new uint8_t[*outSize];
    memset(*binaryStream, 0, *outSize);

    uint8_t temp[4];
    size_t inPos(0), outPos(0);
    for (; base64String[inPos] != '\0'; inPos += 4)
    {
        memset(temp, 0, sizeof(temp));

        temp[0] = mBase64CharIndex_[base64String[inPos]];
        if (!Isbase64(base64String[inPos]))
        {
            return ErrorCode::ec_error;
        }

        temp[1] = mBase64CharIndex_[base64String[inPos + 1]];
        if (!Isbase64(base64String[inPos + 1]))
        {
            return ErrorCode::ec_error;
        }

        (*binaryStream)[outPos++] = static_cast<uint8_t>((temp[0] << 2) & 0xFC) | static_cast<uint8_t>((temp[1] >> 4) & 0x03);
        if (base64String[inPos + 2] == '=')
        {
            break;
        }

        temp[2] = mBase64CharIndex_[base64String[inPos + 2]];
        (*binaryStream)[outPos++] = static_cast<uint8_t>((temp[1] << 4) & 0xF0) | static_cast<uint8_t>((temp[2] >> 2) & 0x0F);
        if (base64String[inPos + 3] == '=')
        {
            break;
        }

        temp[3] = mBase64CharIndex_[base64String[inPos + 3]];
        (*binaryStream)[outPos++] = static_cast<uint8_t>((temp[2] << 6) & 0xF0) | static_cast<uint8_t>(temp[3] & 0x3F);
    }

    *outSize = outPos;
    return ErrorCode::ec_ok;
}

int32_t const MaxDecimalPlaces = 10;

double decPow(int32_t digits)
{
    int di(digits);
    if (di < 0) di = 0;
    if (di > MaxDecimalPlaces) di = MaxDecimalPlaces;

    static const double ExtDecimalArray[] = { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0,1000000000.0,10000000000.0 };
    return ExtDecimalArray[di];
}

double normalizeDouble(double val, int32_t digits)
{
    int di(digits);
    if (di < 0) di = 0;
    if (di > MaxDecimalPlaces) di = MaxDecimalPlaces;

    static const double AdjestArray[] = { 0.51, 0.501, 0.5001, 0.50001, 0.500001, 0.5000001,0.50000001, 0.500000001, 0.5000000001,0.5000000001,0.5000000001 };
    double p = decPow(di);
    return((val >= 0.0) ? (double(int64_t(round(val * p + AdjestArray[di]))) / p) : (double(int64_t(round(val * p - AdjestArray[di]))) / p));
}

int64_t d2IEx(const double val, int32_t digits)
{
    int di(digits);
    if (di < 0) di = 0;
    if (di > MaxDecimalPlaces) di = MaxDecimalPlaces;

    static const double AdjestArray[] = { 0.51, 0.501, 0.5001, 0.50001, 0.500001, 0.5000001,0.50000001, 0.500000001, 0.5000000001,0.5000000001,0.5000000001 };
    //----
    double p = decPow(di);
    return (val >= 0.0) ? static_cast<int64_t>(val * p + AdjestArray[di]) : static_cast<int64_t>(val * p - AdjestArray[di]);
}

string lastErrorToString()
{
    DWORD error = ::GetLastError();
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    string buffer((const char*)lpMsgBuf);

    LocalFree(lpMsgBuf);

    return buffer;
}

bool validEngName(const QString& cid)
{
    if (cid.isEmpty() || cid.length() > CommonConst::MaxNameLength)
    {
        return false;
    }

    static const QRegularExpression exp(R"(^[a-zA-Z][a-zA-Z0-9_]\w*[a-zA-Z0-9]$)");
    QRegularExpressionMatch match = exp.match(cid);
    return match.hasMatch();
}

bool validChsName(const QString& cid)
{
    if (cid.isEmpty() || cid.length() > CommonConst::MaxNameLength)
    {
        return false;
    }

    static const QRegularExpression exp(R"(^[\x{4e00}-\x{9fa5}A-Za-z_][\x{4e00}-\x{9fa5}A-Za-z0-9_]*$)",QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatch match = exp.match(cid);
    return match.hasMatch();
}

void splitCSVLine(const std::string& originalString, std::vector<std::string>& vSubStringGroup)
{
    tokenizer<escaped_list_separator<char>> tok(originalString);
    for (tokenizer<escaped_list_separator<char>>::iterator itor = tok.begin(); itor != tok.end(); ++itor)
    {
        vSubStringGroup.push_back(*itor);
    }
}

void checkJsonChar(char ch, bool& is_backslash, bool& is_quotation, int& num)
{
    if (is_quotation)
    {
        //前一个字符是反斜杠忽略这个字符
        if (is_backslash)
        {
            is_backslash = false;
            return;
        }

        if (ch == CommonConst::Quotation)
        {
            is_quotation = false;
            return;
        }

        if (ch == CommonConst::BackSlash)
        {
            is_backslash = true;
            return;
        }
    }
    else
    {
        switch (ch)
        {
        case CommonConst::Quotation:
        {
            is_quotation = true;
            break;
        }
        case CommonConst::JSONSTART:
        {
            num++;
            break;
        }
        case CommonConst::JSONSEND:
        {
            if (num > 0) { num--; }
            break;
        }
        }
    }
}

bool isAsciiString(const char* str)
{
    if (str == nullptr)
    {
        return true;
    }

    uint32_t nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    uint8_t chr = *str;

    for (uint32_t i = 0; str[i] != '\0'; ++i)
    {
        chr = *(str + i);
        //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
        if (nBytes == 0 && (chr & 0x80) != 0)
        {
            return false;
        }
    }

    return true;
}

bool isUtf8String(const char* str)
{
    if (str == nullptr)
    {
        return true;
    }

    uint32_t nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    uint8_t chr = *str;
    bool bAllAscii = true;

    for (uint32_t i = 0; str[i] != '\0'; ++i)
    {
        chr = *(str + i);
        //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
        if (nBytes == 0 && (chr & 0x80) != 0)
        {
            bAllAscii = false;
        }

        if (nBytes == 0) {
            //如果不是ASCII码,应该是多字节符,计算字节数
            if (chr >= 0x80) {

                if (chr >= 0xFC && chr <= 0xFD)
                {
                    nBytes = 6;
                }
                else if (chr >= 0xF8)
                {
                    nBytes = 5;
                }
                else if (chr >= 0xF0)
                {
                    nBytes = 4;
                }
                else if (chr >= 0xE0)
                {
                    nBytes = 3;
                }
                else if (chr >= 0xC0)
                {
                    nBytes = 2;
                }
                else
                {
                    return false;
                }

                nBytes--;
            }
        }
        else
        {
            //多字节符的非首字节,应为 10xxxxxx
            if ((chr & 0xC0) != 0x80)
            {
                return false;
            }
            //减到为零为止
            nBytes--;
        }
    }

    //违返UTF8编码规则
    if (nBytes != 0)
    {
        return false;
    }

    if (bAllAscii)
    { //如果全部都是ASCII, 也是UTF8
        return true;
    }

    return true;
}

bool CodePageToUnicode(int iCodePage, const char szSrc[], WCHAR szDest[], int& iDestLength)
{
    assert(szSrc);

    int iSize = ::MultiByteToWideChar(iCodePage, 0, szSrc, -1, nullptr, 0);

    if (iSize == 0 || iSize > iDestLength || !szDest || iDestLength == 0)
    {
        iDestLength = iSize;
        return FALSE;
    }

    if (::MultiByteToWideChar(iCodePage, 0, szSrc, -1, szDest, iSize) != 0)
        iDestLength = iSize;
    else
        iDestLength = 0;

    return iDestLength != 0;
}

bool UnicodeToCodePage(int iCodePage, const WCHAR szSrc[], char szDest[], int& iDestLength)
{
    assert(szSrc);

    int iSize = ::WideCharToMultiByte(iCodePage, 0, szSrc, -1, nullptr, 0, nullptr, nullptr);

    if (iSize == 0 || iSize > iDestLength || !szDest || iDestLength == 0)
    {
        iDestLength = iSize;
        return FALSE;
    }

    if (::WideCharToMultiByte(iCodePage, 0, szSrc, -1, szDest, iSize, nullptr, nullptr) != 0)
        iDestLength = iSize;
    else
        iDestLength = 0;

    return iDestLength != 0;
}

std::string UTF8toGBK(const std::string& utf8string)
{
    return UnicodetoGBK(UTF8toUnicode(utf8string));
}

std::string GBKtoUTF8(const std::string& gbkstring)
{
    return UnicodetoUTF8(GBKtoUnicode(gbkstring));
}

std::wstring UTF8toUnicode(const std::string& utf8string)
{
    if (utf8string.empty())
    {
        return L"";
    }

    int ilength(0);
    CodePageToUnicode(CP_UTF8, utf8string.c_str(), nullptr, ilength);

    if (ilength == 0)
    {
        return L"";
    }

    wchar_t* p = new wchar_t[ilength];
    PointerScopeAssistant pa(&p, true);
    if (!CodePageToUnicode(CP_UTF8, utf8string.c_str(), p, ilength))
    {
        return L"";
    }

    wstring unicodestring(p);
    return unicodestring;
}

std::wstring GBKtoUnicode(const std::string& gbkstring)
{
    if (gbkstring.empty())
    {
        return L"";
    }

    int ilength(0);
    CodePageToUnicode(CP_ACP, gbkstring.c_str(), nullptr, ilength);

    if (ilength == 0)
    {
        return L"";
    }

    wchar_t* p = new wchar_t[ilength];
    PointerScopeAssistant pa(&p, true);
    if (!CodePageToUnicode(CP_ACP, gbkstring.c_str(), p, ilength))
    {
        return L"";
    }

    wstring unicodestring(p);
    return unicodestring;
}

std::string UnicodetoUTF8(const std::wstring& unicodestring)
{
    if (unicodestring.empty())
    {
        return "";
    }

    int ilength(0);
    UnicodeToCodePage(CP_UTF8, unicodestring.c_str(), nullptr, ilength);

    if (ilength == 0)
    {
        return "";
    }

    char* p = new char[ilength];
    PointerScopeAssistant pa(&p, true);
    if (!UnicodeToCodePage(CP_UTF8, unicodestring.c_str(), p, ilength))
    {
        return "";
    }

    string utf8string(p);
    return utf8string;
}

std::string UnicodetoGBK(const std::wstring& unicodestring)
{
    if (unicodestring.empty())
    {
        return "";
    }

    int ilength(0);
    UnicodeToCodePage(CP_ACP, unicodestring.c_str(), nullptr, ilength);

    if (ilength == 0)
    {
        return "";
    }

    char* p = new char[ilength];
    PointerScopeAssistant pa(&p, true);
    if (!UnicodeToCodePage(CP_ACP, unicodestring.c_str(), p, ilength))
    {
        return "";
    }

    string gbkstring(p);
    return gbkstring;
}

bool transform_jatosl(const QJsonArray arr,QStringList& sl)
{
    Q_FOREACH(auto elem,arr)
    {
        if(!elem.isString())
        {
            return false;
        }
        sl.push_back(elem.toString());
    }
    return true;
}

std::optional<QStringList> transform_jatosl(const QJsonArray arr)
{
    QStringList sl;
    Q_FOREACH(auto elem,arr)
    {
        if(!elem.isString())
        {
            return std::nullopt;
        }
        sl.push_back(elem.toString());
    }

    return sl;
}

bool readJsonValue(const QJsonObject& jo,const QString& key, double* val)
{
    if(jo.contains(key) && jo[key].isDouble())
    {
        *val = jo[key].toDouble();
        return true;
    }

    return false;
}

bool readJsonValue(const QJsonObject& jo,const QString& key, QColor* val)
{
    if(!jo.contains(key))
    {
        return false;
    }

    const auto& elem = jo[key];
    if(elem.isArray())
    {
        auto colorArray = elem.toArray();
        if(colorArray.size() != 3)
        {
            return false;
        }

        *val = QColor(colorArray[0].toInt(),colorArray[1].toInt(),colorArray[2].toInt());
    }
    else if(elem.isString())
    {
        *val = QColor(elem.toString());
    }
    else
    {
        return false;
    }


    return true;
}

QJsonValue transferType(const QVariant& value)
{
    if(value.isNull())
    {
        return QJsonValue("");
    }

    if(value.type() == QVariant::Bool)
    {
        return QJsonValue(value.toBool());
    }

    QString val = value.toString();
    if((val.length() > 2) && (val[0] == '{') && (val[val.size()-1] == '}'))
    {
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(val.toLocal8Bit(),&error);
        if(error.error == QJsonParseError::NoError)
        {
            return doc.object();
        }
    }
    else if((val.length() > 2) && (val[0] == '"') && (val[val.size()-1] == '"'))
    {
        val = val.mid(1,val.length()-2);
        return QJsonValue(val);
    }

    bool bOk(false);
    auto doubleValue = value.toDouble(&bOk);
    if(bOk)
    {
        return QJsonValue(doubleValue);
    }

    return QJsonValue(val);
}

QJsonValue transferType(const QString& value)
{
    if(value.length() > 2)
    {
        if((value[0] == '{') && (value[value.size()-1] == '}'))
        {
            QJsonParseError error;
            auto doc = QJsonDocument::fromJson(value.toLocal8Bit(),&error);
            if(error.error == QJsonParseError::NoError)
            {
                return doc.object();
            }
        }

        if(value.startsWith('"') && value.endsWith('"'))
        {
            return QJsonValue(value.mid(1,value.length()-2));
        }

        //处理unicode的"",否则实在不好分
        if(value.startsWith(0x201c) && value.endsWith(0x201d))
        {
            return QJsonValue(value.mid(1,value.length()-2));
        }
    }

    if(value.compare("true",Qt::CaseInsensitive)==0)
    {
        return QJsonValue(true);
    }

    if(value.compare("false",Qt::CaseInsensitive)==0)
    {
        return QJsonValue(false);
    }

    bool bOk(false);
    auto doublevalue = value.toDouble(&bOk);
    if(bOk)
    {
        return QJsonValue(doublevalue);
    }

    return QJsonValue(value);
}

}

}


