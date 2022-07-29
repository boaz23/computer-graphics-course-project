//
// Created by hedi1 on 24/01/2022.
//

#ifndef ENIGEREWORK_CAMERA_H
#define ENIGEREWORK_CAMERA_H
#include "Movable.h"
#include <igl/igl_inline.h>
#include <Eigen/Geometry>
#include <Eigen/Core>
namespace igl {
    namespace opengl {
        struct CameraData {
            IGL_INLINE CameraData(float fov, float relationWH, float zNear, float zFar)
                : fov{fov}, relationWH{relationWH}, zNear{zNear}, zFar{zFar} {}

            float fov, relationWH;
            float zNear, zFar;
        };

        class Camera : public Movable {
        public:
            IGL_INLINE Camera();
            IGL_INLINE Camera(CameraData data);

            IGL_INLINE Eigen::Matrix4f GetViewProjection(float displayRatio) const {
                return CalcProjection(displayRatio);
            }

            IGL_INLINE float GetAngle() const {
                return data.fov;
            }

            IGL_INLINE float GetNear() const {
                return data.zNear;
            }

            IGL_INLINE float GetFar() const {
                return data.zFar;
            }

            IGL_INLINE float GetRelationWH() const {
                return data.relationWH;
            }

            IGL_INLINE void SetProjection(float fov, float relationWH);
            IGL_INLINE Eigen::Matrix4f CalcProjection(float relationWH) const;

            IGL_INLINE float CalcMoveCoeff(float depth, int size) const;

            Eigen::Matrix4f _projection;
            CameraData data;
            float length;
            bool _ortho;

        };
    }
}


//#include <igl/serialize.h>
//namespace igl {
//    namespace serialization {
//
//        IGL_INLINE void serialization(bool s, igl::opengl::Camera& obj, std::vector<char>& buffer)
//        {
//
//            SERIALIZE_MEMBER(projection);
//            SERIALIZE_MEMBER(fov);
//            SERIALIZE_MEMBER(relationWH);
//            SERIALIZE_MEMBER(zFar);
//            SERIALIZE_MEMBER(zNear);
//
//        }
//
//        template<>
//        inline void serialize(const igl::opengl::Camera& obj, std::vector<char>& buffer)
//        {
//            serialization(true, const_cast<igl::opengl::Camera&>(obj), buffer);
//        }
//
//        template<>
//        inline void deserialize(igl::opengl::Camera& obj, const std::vector<char>& buffer)
//        {
//            serialization(false, obj, const_cast<std::vector<char>&>(buffer));
//        }
//    }
//}

#endif //ENIGEREWORK_CAMERA_H
