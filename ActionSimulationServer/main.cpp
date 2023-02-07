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

#include <QCoreApplication>
#include <QTextCodec>
#include "actionsimulationserver.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QTextCodec * utf8 = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(utf8);

    if (!gActionSimulationServer.loadConfiguration())
    {
        LOGERROR(QStringLiteral("%1:%2").arg(__FILE__).arg(__LINE__));
        return Jimmy::ErrorCode::ec_error;
    }

    ActionSimulationService service(argc, argv);

    if(argc > 1)
    {
        QString params(argv[1]);
        if((params.compare("-e")==0)||(params.compare("-exec")==0))
        {
            GlobalLogger::get_instance()->sendConsole(true);
        }
    }

    return service.exec();
}
