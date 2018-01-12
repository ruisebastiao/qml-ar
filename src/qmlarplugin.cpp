#include "qmlarplugin.h"

void QMLARQMLPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("AR"));
    qmlRegisterSingletonType<QMLAR>("AR", 1, 0, "AR", get_qml_ar);
}

QObject *QMLARQMLPlugin::get_qml_ar(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)

    QMLAR* qml_ar = new QMLAR();
    engine->addImageProvider("ARMarkers", qml_ar->getImageProvider());
    return qml_ar;
}