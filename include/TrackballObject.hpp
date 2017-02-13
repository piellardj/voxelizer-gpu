#ifndef TRACKBALLOBJECT_HPP_INCLUDED
#define TRACKBALLOBJECT_HPP_INCLUDED


#include "glm.hpp"


/**@brief Trackball object base class.
 *
 * The object rotates around a fixed 3D point.
 * A position is defined by a latitude, a longitude and a distance from the focus point.
 */
class TrackballObject
{
    public:
        TrackballObject (glm::vec3 const& focusPoint,
                         float distance,
                         float latitude,
                         float longitude);
        virtual ~TrackballObject() {}

        void setFocusPoint (glm::vec3 focusPoint);
        glm::vec3 const& getFocusPoint () const;

        /**@arg distance Clamped to [0, +inf[ */
        void setDistance (float distance);
        float getDistance () const;

        /**@arg latitude In radians */
        void setLatitude (float latitude);
        float getLatitude () const;

        /**@arg longitude In radians, between [-PI/2, PI/2], clamped otherwise */
        void setLongitude (float longitude);
        float getLongitude () const;

        /**@brief Returns the cartesian coordinates relative to the focus point. */
        glm::vec3 getRelativePosition () const;

        /**@brief Returns the absolute cartesian coordinates. */
        glm::vec3 getWorldPosition () const;

    protected:
        virtual void positionChanged () {}


    private:
        glm::vec3 _focusPoint;
        float _distance;
        float _latitude;
        float _longitude;
};

#endif // TRACKBALLOBJECT_HPP_INCLUDED
