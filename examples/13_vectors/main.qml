/**
 * @file main.qml
 * @brief Draw spheres on the scene with your finger
 * @author Sergei Volodin
 * @version 1.0
 * @date 2018-08-03
 */

import QtQuick.Window 2.2
import ch.epfl.chili.qmlar 1.0
import QtQuick 2.6
import QtQuick.Controls 2.3


/** @brief Draw spheres on the scene with your finger */
Window {
    // some window parameters
    visible: true
    height: 500
    width: 500
    id: root

    Text {
        anchors.top: parent.top
        text: "Click on the scene to add a sphere at that point"
    }

    Button {
        text: "Clear"
        anchors.top: parent.top
        anchors.right: parent.right
        onClicked: arComponent.clearSpheres()
    }

    // crearing AR component
    ARComponent {
        id: arComponent

        // loading scene
        arSceneComponent: Qt.createComponent("Activity.qml");

        // no menu in this demo
        //disable_menu: true

        // setting width
        width: 500

        // added spheres
        property var spheres: ([]);

        property vector2d clickPoint: Qt.vector2d(0, 0)
        property int selected: -1

        // do an action when clicked on the plane with the markers
        onClickedOnActivity: {
            var component = Qt.createComponent("SphereEntity.qml");
            var obj = component.createObject(arSceneObject, {'x': x_mm, 'y': y_mm})
            spheres.push(obj);
            mouseHover = true;

            var z_mm = 0;
            var closest_i = -1;
            var found = 0;

            var vec = Qt.vector3d(x_mm, y_mm, z_mm);
            var threshold = 5;

            var demo = arSceneObject.d;

            for(var i = 0; i < demo.lst.length; i++)
            {
                var arrow = demo.lst[i];
                var from = arrow.lvector.from;
                var to = arrow.lvector.to;
                console.log(from.minus(vec).length());
                if(from.minus(vec).length() <= threshold)
                {
                    console.log("Clicked at FROM", i);
                }
                else if(to.minus(vec).length() <= threshold)
                {
                    console.log("Clicked at TO", i);
                    clickPoint = Qt.vector2d(x_mm, y_mm)
                    selected = i
                }
            }
        }

        onMovedOnActivity: {
            var delta = Qt.vector3d(x_mm - clickPoint.x, y_mm - clickPoint.y, 0);
            if(selected >= 0)
            {
                arSceneObject.d.lst[selected].lvector.to = arSceneObject.d.lst[selected].lvector.to.plus(delta);
                arSceneObject.d.lst[selected].lvector.from = arSceneObject.d.lst[selected].lvector.from.plus(delta);
            }

            clickPoint = Qt.vector2d(x_mm, y_mm)
            //arSceneObject.d.lst[0].lvector.to = Qt.vector3d(x_mm, y_mm, 0);
        }

        function clearSpheres() {
            for(var i = 0; i < spheres.length; i++) {
                var sphere = spheres[i];
                sphere.destroy();
            }
            spheres = [];
        }

        init_type: AR.INIT_IMAGE
        image_filename: "://assets/ar_demo_sheet.png"
    }
}
