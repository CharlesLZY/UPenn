#include "player.h"
#include <QString>
#include <iostream>
#include <chrono>
#include <thread>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      ray_axis(-1), mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

// Update the Player's velocity and acceleration based on the
// state of the inputs.
void Player::processInputs(InputBundle &inputs) {
    // this variable makes sure to halt the movement
    m_acceleration = glm::vec3(0.f);
    bool nothingClicked = true;
    if (inputs.flight_mode) {
        if (inputs.wPressed) {
            m_acceleration += m_forward;
        } else if (inputs.sPressed) {
            m_acceleration += -m_forward;
        } else if (inputs.dPressed) {
            m_acceleration += m_right;
        } else if (inputs.aPressed) {
            m_acceleration += -m_right;
        } else if (inputs.ePressed) {
            m_acceleration.y += m_up.y;
        } else if (inputs.qPressed) {
            m_acceleration.y += -m_up.y;
        } else {
            m_velocity = glm::vec3(0.f);
            m_acceleration = glm::vec3(0.f);
        }
        if (m_acceleration != glm::vec3(0.f)) {
            m_acceleration = glm::normalize(m_acceleration);
        }
    } else {
        if (inputs.wPressed) {
            m_acceleration += glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
            nothingClicked = false;
        }
        if (inputs.sPressed) {
            m_acceleration += -glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
            nothingClicked = false;
        }
        if (inputs.aPressed) {
            m_acceleration += -glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
            nothingClicked = false;
        }
        if (inputs.dPressed) {
            m_acceleration += glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
            nothingClicked = false;
        }
        if (inputs.spacePressed) {
            if (isOnGround(mcr_terrain, inputs) && !(getCameraBlock(mcr_terrain) == WATER) && !(getCameraBlock(mcr_terrain) == LAVA)) {
                m_velocity.y = 30.f * m_up.y;
                nothingClicked = false;
            }
            else if (getCameraBlock(mcr_terrain) == WATER || getCameraBlock(mcr_terrain) == LAVA) {
                m_velocity.y += 5.f * m_up.y;
                nothingClicked = false;
            }
        }
        if (nothingClicked){
            // we don't want to interfere with the y axis
            // so that the player can complete the motion of jumping
            m_acceleration.x = 0.f;
            m_acceleration.z = 0.f;
            m_velocity.x = 0.f;
            m_velocity.z = 0.f;
        }
    }
}

// Update the Player's position based on its acceleration
// and velocity, and also perform collision detection.
void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // the player's velocity is reduced to less than 100% of its current value
    // every frame (simulates friction + drag) before acceleration is added to it.
    glm::vec3 gravity = glm::vec3(0.0f, -9.8f, 0.0f) * 10.f;
    m_velocity += m_acceleration * dT * 30.f;
    m_velocity *= 0.9f;
    glm::vec3 rayDirection = m_velocity + dT * m_acceleration;
    // only perform collision detection when you are not in flight mode
    if (!inputs.flight_mode) {
        if (!isOnGround(terrain, inputs)) {
            if ((getCameraBlock(terrain) == WATER || getCameraBlock(terrain) == LAVA)) {
                if (!inputs.spacePressed) {
                    // drop to the ground
                    gravity *= 0.3;
                    m_acceleration += gravity;
                    m_velocity += m_acceleration * dT;
                }
            } else {
                // drop to the ground
                m_acceleration += gravity;
                m_velocity += m_acceleration * dT;
            }
        } else if (isOnGround(terrain, inputs)){
            m_acceleration.y = 0.f;
            m_velocity.y = glm::max(m_velocity.y, 0.f);
        }
        rayDirection = m_velocity * dT;
        detectCollision(&rayDirection, terrain);
        if (getCameraBlock(terrain) == WATER || getCameraBlock(terrain) == LAVA || getPositionBlock(terrain) == WATER) {
            // the player should move at 2/3 its normal speed
            rayDirection *= 0.4;
        }

    } else {
         rayDirection *= 0.2;
         // sound effect for flight mode
//         if (effect.source() != QUrl("qrc:/sound/wind.wav")) {
//             effect.stop();
//             effect.setSource(QUrl("qrc:/sound/wind.wav"));
//             effect.setLoopCount(100);
//             effect.setVolume(10.f);
//             effect.play();
//         }
    }
    this->moveAlongVector(rayDirection);
}

bool Player::isOnGround(const Terrain &terrain, InputBundle &inputs) {
    // player position is 0.5 away from the vertex
    // chose bottomleft because I wanted to add 1 to get all the other three vertices
    glm::vec3 bottomLeft = m_position - glm::vec3(0.5f, 0, 0.5f);
    // traverse the four bottom vertices
    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            // the offset for y should be 0.005f, but for some reason it doesnt work
            // so I had to increase the offset for it to detect that it is on the ground
            glm::vec3 vertexPos = glm::vec3(floor(bottomLeft.x) + x, floor(bottomLeft.y - 0.1f),
                    floor(bottomLeft.z) + z);
            // as long as one of the vertex is on a block that is not empty
            // player is on the ground
            BlockType currBlock = terrain.getBlockAt(vertexPos);
            if (currBlock != EMPTY && currBlock != WATER && currBlock != LAVA) {
                inputs.isOnGround = true;
                if (!inputs.spacePressed) {
                    m_acceleration.y = 0.f;
                    m_velocity.y = 0.f;
                }
                return true;
            }
        }
    }
    inputs.isOnGround = false;
    return false;
}

// copied directly from the slide
bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        ray_axis = interfaceAxis;
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if (cellType != EMPTY && cellType != WATER && cellType != LAVA) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    // TA said I need to offset by a very small number to avoid bugs
    *out_dist = glm::min(maxLen, curr_t) - 0.00001;
    return false;
}

void Player::detectCollision(glm::vec3 *rayDirection, const Terrain &terrain) {
    glm::vec3 bottomLeft = this->m_position - glm::vec3(0.5f, 0.f, 0.5f);
    glm::ivec3 out_blockHit = glm::ivec3();
    float out_dist = 0.f;

    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            for (int y = 0; y < 3; y++) {
                // doing them each axis individually in order to move along the wall instead of just halt
                glm::vec3 rayOrigin = bottomLeft + glm::vec3(x, y, z);
                glm::vec3 x_ray = glm::vec3(rayDirection->x, 0.f, 0.f);
                glm::vec3 y_ray = glm::vec3(0.f, rayDirection->y, 0.f);
                glm::vec3 z_ray = glm::vec3(0.f, 0.f, rayDirection->z);
                if (gridMarch(rayOrigin, x_ray, terrain, &out_dist, &out_blockHit)) {
                    // this offset is necessary, but it is not letting the player land after jumping (problem fixed)
                    float distance = glm::min(out_dist - 0.005f, glm::abs(glm::length(this->m_position - glm::vec3(out_blockHit))));
                    x_ray = distance * glm::normalize(x_ray);
                }
                if (gridMarch(rayOrigin, y_ray, terrain, &out_dist, &out_blockHit)) {
                    float distance = glm::min(out_dist - 0.005f, glm::abs(glm::length(this->m_position - glm::vec3(out_blockHit))));
                    y_ray = distance * glm::normalize(y_ray);
                }
                if (gridMarch(rayOrigin, z_ray, terrain, &out_dist, &out_blockHit)) {
                    float distance = glm::min(out_dist - 0.005f, glm::abs(glm::length(this->m_position - glm::vec3(out_blockHit))));
                    z_ray = distance * glm::normalize(z_ray);
                }
                // combine the three axis
                *rayDirection = glm::vec3(x_ray.x, y_ray.y, z_ray.z);
            }
        }
    }
}



// returns the block that the camera position is at
BlockType Player::getCameraBlock(const Terrain &terrain) {
    glm::vec3 cam_pos = m_camera.mcr_position;
    // we have to add hasChunkAt() because initially when the terrain is still forming, there are no chunks
    if (cam_pos.y > 255 || cam_pos.y < 0 || !terrain.hasChunkAt(floor(cam_pos.x), floor(cam_pos.z))) {
        return EMPTY;
    }

    BlockType cam_block = terrain.getBlockAt(floor(cam_pos.x), floor(cam_pos.y), floor(cam_pos.z));

    return cam_block;
}

BlockType Player::getPositionBlock(const Terrain &terrain) {
    glm::vec3 pos = m_position - glm::vec3(0.f, 0.5f, 0.f);
    // we have to add hasChunkAt() because initially when the terrain is still forming, there are no chunks
    if (pos.y > 255 || pos.y < 0 || !terrain.hasChunkAt(floor(pos.x), floor(pos.z))) {
        return EMPTY;
    }

    BlockType pos_block = terrain.getBlockAt(floor(pos.x), floor(pos.y), floor(pos.z));

    return pos_block;
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
