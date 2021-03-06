/**
 * @file markerstorage.cpp
 * @brief This class stores the set of used markers
 * loads them from a file and outputs
 * the coordinates of each of them
 * on a request
 * @author Sergei Volodin
 * @version 1.0
 * @date 2018-07-25
 */

#include "markerstorage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "timelogger.h"

MarkerStorage::MarkerStorage()
{
    markers.clear();
}

MarkerStorage& MarkerStorage::operator =(const MarkerStorage& that)
{
    this->markers = that.markers;
    return *this;
}

bool MarkerStorage::markersDetected()
{
    return getCorrespondences().size() > 0;
}

MarkerStorage::MarkerStorage(const MarkerStorage &that) : QObject(), ConfigJSON()
{
    this->markers = that.markers;
}

void MarkerStorage::readConfig(QJsonObject config)
{
    QJsonArray markers_obj = config.value("markers").toArray();
    for(QJsonArray::iterator it = markers_obj.begin(); it != markers_obj.end(); it++)
    {
        QJsonObject marker = (*it).toObject();
        QString name = marker.value("name").toString();

        int id = marker.value("id").toInt();
        double x_mm = marker.value("x_mm").toDouble();
        double y_mm = marker.value("y_mm").toDouble();
        double size_mm = marker.value("size_mm").toDouble();

        markers[id] = Marker(x_mm, y_mm);
        markers[id].setName(name);;
        markers[id].setId(id);
        markers[id].setSizeMM(size_mm);
        markers[id].setConfig(marker);

        TimeLoggerLog("%s", markers[id].toString().toStdString().c_str())
    }
}

void MarkerStorage::undetect()
{
    QMap<int, Marker>::iterator it;
    for(it = markers.begin(); it != markers.end(); it++)
        (*it).undetect();
}

Marker* MarkerStorage::getPointer(int marker_id)
{
    if(markers.contains(marker_id))
        return(&markers[marker_id]);
    else
    {
        TimeLoggerLog("Marker %d does not exist in storage", marker_id);
        return(NULL);
    }
}

Marker MarkerStorage::get(int marker_id)
{
    Marker* marker = getPointer(marker_id);
    if(marker == NULL)
    {
        qFatal("Marker %d does not exist in storage", marker_id);
        return Marker();
    }
    return *marker;
}

QMap<int, Marker>::iterator MarkerStorage::begin()
{
    return markers.begin();
}

QMap<int, Marker>::iterator MarkerStorage::end()
{
    return markers.end();
}

WorldImageCorrespondences MarkerStorage::getCorrespondences()
{
    WorldImageCorrespondences result;
    result.clear();

    QMap<int, Marker>::iterator it;
    for(it = markers.begin(); it != markers.end(); it++)
        result.join((*it).getCorrespondences());

    return result;
}
