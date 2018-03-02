#include "timelogger.h"
#include "blackenrest.h"
#include <QPainter>
#include <QtCore>
#include <QPen>
#include <QColor>
#include <QRect>

BlackenRest::BlackenRest(PosePredictor *predictor) : ImageProviderAsync()
{
    Q_ASSERT(predictor != NULL);

    // saving arguments
    this->predictor = predictor;

    // initially, not using the region
    use_region = false;
}

void BlackenRest::setInput(PipelineContainer<QImage> img)
{
    object_in_process = img.info();
    TimeLoggerLog("%s", "[ANALYZE] Begin Blacken");
    QImage blackened = blacken(img.o());
    TimeLoggerLog("%s", "[ANALYZE] End Blacken");

    emit imageAvailable(PipelineContainer<QImage>
                        (blackened,object_in_process.checkpointed("BlackenRest")));
}

void BlackenRest::onNewPMatrix(PipelineContainer<QMatrix4x4> p)
{
    P = p.o();
}

void BlackenRest::onNewMVMatrix(PipelineContainer<QMatrix4x4> mv)
{
    predictor->setCurrentPose(mv.o());
}

void BlackenRest::onNewMarkers(PipelineContainer<MarkerStorage> storage)
{
    use_region = storage.o().markersDetected();
    this->storage = storage.o();
}

QImage BlackenRest::blacken(QImage source)
{
    if(!use_region)
        return source;

    TimeLoggerLog("%s", "Obtaining marker location");
    // obtaining predicted pose
    Pose predictedPose = predictor->predictPose();

    // obtaining new MVP
    QMatrix4x4 MVP = P * predictedPose.get4Matrix();

    // obtaining image correspondences
    WorldImageCorrespondences correspondences = storage.getCorrespondences();

    // resulting polygon containing marker
    QPolygon marker;

    // mapping correspondences to image coordinate system
    for(int i = 0; i < correspondences.size(); i++)
    {
        // original 3D world point
        QVector3D world_point = correspondences.getWorldPoint(i);
        QVector4D world_point_4d = QVector4D(world_point);
        world_point_4d.setW(1);

        // predicted by predictor image point
        QVector4D predicted_image_point_4d = MVP.map(world_point_4d);
        QVector3D predicted_image_point = predicted_image_point_4d.toVector3D();

        // affine transform
        predicted_image_point.setX(predicted_image_point.x() / predicted_image_point.z());
        predicted_image_point.setY(predicted_image_point.y() / predicted_image_point.z());

        // OpenGL NDC -> image coordinates
        predicted_image_point.setX((predicted_image_point.x() + 1) / 2. * source.width());
        predicted_image_point.setY((1 - predicted_image_point.y()) / 2. * source.height());

        // adding point as marker corner
        marker.append(QPoint(predicted_image_point.x(), predicted_image_point.y()));
    }

    // blackened output
    QImage augmented_input = source;

    TimeLoggerLog("%s", "Blackening input");

    // removing all but the selected rectangle
    if(marker.size() > 0)
    {
        // copying the image
        QPainter p(&augmented_input);

        // all image selection
        QPainterPath pp_all;
        pp_all.addRect(0, 0, source.width(), source.height());

        // marker selection
        QPainterPath pp;
        pp.addPolygon(marker);

        // want to blacken
        p.setBrush(QBrush(Qt::black));

        // blackening all but marker
        p.drawPath(pp_all - pp);

        // returning resulting image
        return augmented_input;
    }

    return source;
}
