#include "endpoint.h"

Endpoint::Endpoint(const QString &_route, const QString &_method)
    : route(_route), method(_method)
{

}

void Endpoint::setMethod(const QString &_method){
    method = _method;
}

void Endpoint::setRoute(const QString &_route){
    route = _route;
}

const QString &Endpoint::getMethod(){
    return method;
}

const QString &Endpoint::getRoute(){
    return route;
}
