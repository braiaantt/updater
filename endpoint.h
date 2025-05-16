#ifndef ENDPOINT_H
#define ENDPOINT_H
#include <qstring.h>

class Endpoint
{
public:
    Endpoint(const QString&, const QString&);
    //getters
    const QString &getRoute() const;
    const QString &getMethod() const;

    //setters
    void setRoute(const QString &);
    void setMethod(const QString &);

private:
    QString route;
    QString method;
};

#endif // ENDPOINT_H
