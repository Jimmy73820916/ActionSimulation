#ifndef NODALPOINT_H
#define NODALPOINT_H

#include<QString>
#include<QList>

namespace Jimmy
{
namespace MaxMatchTree
{

class NodalPoint
{
public:
    NodalPoint(QChar id);
    ~NodalPoint();

    void append(const QString& leaf);
    QString findMatch(const QString& heads);
private:
    QChar getID() { return id_;}
    QString getData() { return Data_;}
    void setData(const QString& data) { Data_ = data;}
    void append(int level,const QString& leaf);
    void appendorset(int level,const QString& leaf);
    QString findMatch(int level,const QString& heads);
    QString findMaxMatch(bool ishead,QString& match);
private:
    QList<NodalPoint*> children_;
    QChar id_;
    QString Data_;
};

}
}


#endif // NODALPOINT_H
