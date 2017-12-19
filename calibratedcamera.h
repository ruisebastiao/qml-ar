#ifndef CALIBRATEDCAMERA_H
#define CALIBRATEDCAMERA_H

#include <QMatrix3x3>

/*
 * This class stores information
 * about a calibrated camera
 */

class CalibratedCamera
{
protected:
    QMatrix3x3 A;

public:
    CalibratedCamera();
    QMatrix3x3 getMatrix();

    QMatrix4x4 getPerspectiveMatrix(float n, float f, float l, float r, float b, float t);
    QMatrix4x4 getPerspectiveMatrixMethod1(float n, float f, float l, float r, float b, float t);
    QMatrix4x4 getPerspectiveMatrixMethod2(float n, float f, float l, float r, float b, float t);
};

#endif // CALIBRATEDCAMERA_H