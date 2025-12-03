#pragma once
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/quaternion.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

// ============================================
// 좌표계 변환 헬퍼 함수
// ============================================
// FBX: Y-up, Z-forward
// OpenGL: Y-up, Z-backward
// 변환: scale(1, 1, -1)을 사용한 축 변환

// Axis fix matrix (Z축 반전)
inline glm::mat4 AxisFixMatrix() {
    return glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
}

// aiMatrix4x4를 glm::mat4로 변환
inline glm::mat4 AiMatrixToGlm(const aiMatrix4x4& a) {
    glm::mat4 m;
    m[0][0] = a.a1; m[1][0] = a.a2; m[2][0] = a.a3; m[3][0] = a.a4;
    m[0][1] = a.b1; m[1][1] = a.b2; m[2][1] = a.b3; m[3][1] = a.b4;
    m[0][2] = a.c1; m[1][2] = a.c2; m[2][2] = a.c3; m[3][2] = a.c4;
    m[0][3] = a.d1; m[1][3] = a.d2; m[2][3] = a.d3; m[3][3] = a.d4;
    return m;
}

// aiMatrix4x4를 OpenGL 좌표계로 변환
// fix * matrix * fix 공식 사용
inline glm::mat4 ConvertAiMatToGlmFixed(const aiMatrix4x4& a) {
    glm::mat4 gm = AiMatrixToGlm(a);
    glm::mat4 fix = AxisFixMatrix();
    return fix * gm * fix;
}

// TRS 컴포넌트를 FBX 좌표계에서 OpenGL 좌표계로 변환
// fix * TRS * fix 공식 사용
inline glm::mat4 BuildAndFixTRS(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 R = glm::mat4_cast(rot);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 trs = T * R * S;
    glm::mat4 fix = AxisFixMatrix();
    return fix * trs * fix;
}
