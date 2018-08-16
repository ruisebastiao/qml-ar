/**
 * @file qtbackend.cpp
 * @brief QCamera backend for QML
 * @author Sergei Volodin
 * @version 1.0
 * @date 2018-07-25
 */

#include "timelogger.h"
#include "qtbackend.h"
#include "qvideoframehelpers.h"
#include <QtMultimedia/QCameraInfo>
#include <QVideoProbe>
#include <QException>
#include "qvideoframehelpers.h"
#include "voidviewfinder.h"
#include "nv21videofilter.h"
#include "nv21videofilterrunnable.h"

QtCameraBackend::QtCameraBackend(int cam_id) : ImageProviderAsync()
{
    // initializing camera
    TimeLoggerLog("Number of cameras: %d", QCameraInfo::availableCameras().size());
    camera = new QCamera(QCameraInfo::availableCameras().at(cam_id));
    camera->setParent(this);
    watcher.setParent(this);
    need_viewfinder = 1;
    frame_available = 0;
    image_id = 0;
    use_gpu = true;
    init();
}

void QtCameraBackend::init()
{
    // initial buffer value is an empty image
    buf = QVideoFrameHelpers::empty();

    // installing camera callback
    // has different implementations for Android/Linux
    // since Android camera is poorly supported in Android NDK (Nov'17)

    // android callback will be installed in start()
#if defined Q_OS_LINUX && !defined Q_OS_ANDROID && !defined QT_BACKEND_FORCE_VIDEOPROBE
    // using custom class as a viewfinder
    frameGrabber = new CameraFrameGrabber();
    camera->setViewfinder(frameGrabber);

    // installing callback
    connect(frameGrabber, SIGNAL(frameAvailable(QImage)), this, SLOT(processQImage(QImage)));
#endif

    // waiting for async output
    connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
}

void QtCameraBackend::start()
{
#if defined Q_OS_ANDROID || defined QT_BACKEND_FORCE_VIDEOPROBE
    // setting up a viewfinder which does nothing
    //    if(need_viewfinder)
    //    {
    //        viewfinder = new VoidViewFinder;
    //        camera->setViewfinder(viewfinder);
    //    }

    // this object does image grabbing
    probe = new QVideoProbe(this);

    // installing callback
    if(probe->setSource((QMediaObject *) camera))
        connect(probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(processQVideoFrame(const QVideoFrame &)));
    else
        qFatal("Can't connect probe to camera");
#endif

    // starting camera thread
    camera->start();
}

QImage QtCameraBackend::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{ Q_UNUSED(id) Q_UNUSED(size) Q_UNUSED(requestedSize)
            return buf;
}

QtCameraBackend::~QtCameraBackend() {
}

void QtCameraBackend::processAndSendQImage(PipelineContainer<QImage> img)
{
    buf = img.o();

    // sending image
    emit imageAvailable(PipelineContainer<QImage>
                        (img.o(), img.info().checkpointed("Camera")));

    TimeLoggerThroughput("%s", "[ANALYZE] Begin QtCamera");
}

void QtCameraBackend::processQImage(QImage img)
{
    buf = img;
}

QCamera *QtCameraBackend::getCamera()
{
    return camera;
}

void QtCameraBackend::handleFinished()
{
    // saving to buffer
    processQImage(watcher.result());

    // sending image
    emit imageAvailable(PipelineContainer<QImage>
                        (buf.copy(), image_info.checkpointed("Camera")));

    TimeLoggerThroughput("%s", "[ANALYZE] Begin QtCamera");
}

void QtCameraBackend::processQVideoFrame(const QVideoFrame &frame)
{
    // doing nothing if deactivated
    if(!is_active) return;

    TimeLoggerThroughput("%s", "Received image from camera");

    qDebug() << "Processing QVideoFrame";

    // fast GPU-based processing on Android 26 and higher
#if __ANDROID_API__ >= 26
    // at some point viewfinder and filters should be ready, so it's safe to dereference pointer to the videofilter
    NV21VideoFilterRunnable* runnable = NV21VideoFilter::runnable;
    if(runnable && use_gpu) {
        // switching to videofilter output and disabling videoprobe
        probe->setSource((QMediaObject*) nullptr);
        disconnect(probe, SIGNAL(videoFrameProbed(const QVideoFrame &)), this, SLOT(processQVideoFrame(const QVideoFrame &)));
        connect(runnable, &NV21VideoFilterRunnable::imageConverted, this, &QtCameraBackend::processAndSendQImage, Qt::QueuedConnection);
        qDebug() << "Switched to VideoFilterInput";
        return;
    }
#endif

    // not converting frame if thread is busy
    if(!watcher.isRunning())
    {
        image_info = PipelineContainerInfo(image_id);
        image_id++;
        image_info.checkpoint("Grabbed");
        QFuture<QImage> future = QtConcurrent::run(&QVideoFrameHelpers::VideoFrameToImage, frame);
        watcher.setFuture(future);
    }
}
