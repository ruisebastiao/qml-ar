import QtQuick 2.0

// holds vector with a starting point
Item {
    id: root

    // starting point
    property vector3d from: to.minus(vector)

    // end point
    property vector3d to: from.plus(vector)

    // difference end - start
    property vector3d vector: to.minus(from)

    // only x and y as vector2d
    property vector2d xypart: Qt.vector2d(vector.x, vector.y)

    // vector3d as string
    function vec2str(vec) {
        return '(' + vec.x + ', ' + vec.y + ', ' + vec.z + ')';
    }

    // localized vector as string
    property string text: vec2str(from) + "->" + vec2str(to)
}