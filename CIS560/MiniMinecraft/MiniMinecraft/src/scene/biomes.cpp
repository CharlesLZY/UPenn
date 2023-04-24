#include "biomes.h"
#include <iostream>

/*
Milestone 1
The terrain should fill the space from Y = 0 to Y = 128 entirely with STONE blocks.
When 128 < Y <= 255, the terrain should be filled with blocks of a type dependent on the biome and up to a height dictated by the biome's height field.
In the grassland biome, each column should be filled with DIRT except for the very top block, which should be GRASS.
In the mountain biome, you should fill each column with more STONE, but if the column rises above Y = 200 then the very top block in that column should be SNOW. In both biomes, any EMPTY blocks that fall between a height of 128 and 138 should be replaced with WATER.
In both biomes, any EMPTY blocks that fall between a height of 128 and 138 should be replaced with WATER.
*/


float random1(glm::vec2 p) {
    return glm::fract(sin(glm::dot(p, glm::vec2(127.1,311.7)))*43758.5453);
}

glm::vec2 random2(glm::vec2 p) {
    return glm::vec2(glm::fract(sin(glm::dot(p, glm::vec2(127.1, 311.7))) * 43758.5453),
                     glm::fract(sin(glm::dot(p, glm::vec2(269.5, 183.3))) * 43758.5453));
}

glm::vec2 rotate(glm::vec2 p, float deg) {
    float rad = deg * 3.14159 / 180.0;
    return glm::vec2(glm::cos(rad) * p.x - glm::sin(rad) * p.y,
                     glm::sin(rad) * p.x + glm::cos(rad) * p.y);
}

float WorleyNoise(glm::vec2 uv) {
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0;
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(uvInt + neighbor);
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

float surflet(glm::vec2 P, glm::vec2 gridPoint) {
    float distX = glm::abs(P.x - gridPoint.x);
    float distY = glm::abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);
//    glm::vec2 gradient = random2(gridPoint) * 2.f - glm::vec2(1.f, 1.f);
    glm::vec2 gradient = random2(gridPoint);

    glm::vec2 diff = P - gridPoint;
//    float height = (glm::dot(diff, gradient) + 1.f) * 0.5;
    float height = glm::dot(diff, gradient);
    return height * tX * tY;
}


float PerlinNoise(glm::vec2 uv) {
    glm::vec2 uvXLYL = glm::floor(uv);
    glm::vec2 uvXHYL = uvXLYL + glm::vec2(1,0);
    glm::vec2 uvXHYH = uvXLYL + glm::vec2(1,1);
    glm::vec2 uvXLYH = uvXLYL + glm::vec2(0,1);
    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}


glm::vec2 hash(glm::vec2 p) {
    p = glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)), glm::dot(p, glm::vec2(269.5, 183.3)));
    glm::vec2 fract = glm::fract(glm::vec2(53758.5453123 * glm::cos(p.x), 53758.5453123 * glm::cos(p.y)));
    return glm::vec2(-1.0 + 2.0 * fract.x, -1.0 + 2.0 * fract.y);
}

float step(float edge, float x) {
    return x < edge ? 0.f : 1.f;
}

float SimplexNoise(glm::vec2 p) {
    float K1 = 0.366025404; // (sqrt(3)-1)/2;
    float K2 = 0.211324865; // (3-sqrt(3))/6;

    glm::vec2  i = glm::floor(p + (p.x + p.y) * K1);
    glm::vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(float(a.y), float(a.x));
    glm::vec2  o = glm::vec2(m, 1.0 - m);
    glm::vec2  b = a - o + K2;
    glm::vec2  c = glm::vec2(a.x - 1.0 + 2.0 * K2, a.y - 1.0 + 2.0 * K2);
    glm::vec3  h = glm::max(glm::vec3(0.5) - glm::vec3(glm::dot(a,a), glm::dot(b,b), glm::dot(c,c)), glm::vec3(0.0));
    glm::vec3  n = h*h*h*h * glm::vec3(glm::dot(a, hash(i+glm::vec2(0.0))), glm::dot(b,hash(i+o)), glm::dot(c, hash(i+glm::vec2(1.0))));
    return glm::dot(n, glm::vec3(70.0));
}

float bilerpNoise(glm::vec2 uv) {
    glm::vec2 uvFract = glm::fract(uv);
    float ll = random1(glm::floor(uv));
    float lr = random1(glm::floor(uv) + glm::vec2(1,0));
    float ul = random1(glm::floor(uv) + glm::vec2(0,1));
    float ur = random1(glm::floor(uv) + glm::vec2(1,1));
    float lerpXL = glm::mix(ll, lr, uvFract.x);
    float lerpXU = glm::mix(ul, ur, uvFract.x);
    return glm::mix(lerpXL, lerpXU, uvFract.y);
}

float fbm(glm::vec2 uv) {
    int octave = 8;
    float persistence = 0.5;
    float freq = 1.0;
    float sum = 0.0;
    for(int i = 0; i < octave; i++) {
        float amp = pow(persistence, i);
        sum += bilerpNoise(uv * freq) * amp;
        freq *= 2.0;
    }
    return sum;
}


float ridge(glm::vec2 uv) {
    float WARP_FREQUENCY = 8.0;
    float WARP_MAGNITUDE = 2.0;
    float NOISE_FREQUENCY = 4.0;
    float SCALE = 1000.0;
    uv = uv / SCALE;
    float nx = PerlinNoise(uv * WARP_FREQUENCY);
    float ny = PerlinNoise(rotate(uv * WARP_FREQUENCY, 60.0));
    glm::vec2 warp = glm::vec2(nx, ny) * WARP_MAGNITUDE;
    uv = uv * NOISE_FREQUENCY + warp;
    // Worley Noise
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0;
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(uvInt + neighbor);
            point = glm::vec2(0.5 + 0.5 * glm::sin(52768 * 0.01 + 6.2831 * point.x),
                              0.5 + 0.5 * glm::sin(52768 * 0.01 + 6.2831 * point.y));
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    float res = minDist < 0.5 ? minDist * minDist + 0.25 : minDist;
    res = glm::smoothstep(0.1f, 1.f, minDist);
    return res * 0.5 + 0.5;
}

//float mountain(glm::vec2 uv) {
//    float SCALE = 100.0;
//    uv = uv / SCALE;
//    glm::vec2 coord = uv;
//    glm::mat2 m = glm::mat2( 1.3, 2.2, -1.4, 1.2 );
//    float h  = 0.5000 * SimplexNoise( coord );
//    coord = m*coord;
//    //  bug here h can be negative
//    h += 0.2500*SimplexNoise( coord );
////    h = 0.5 * (h + 1.f);
////    std::cout<<(h>1)<<std::endl;
////    return h * (250 - 128) + 128;
//    return glm::clamp(h, 0.f, 1.f) * (250 - 128) + 128;
//}

float desert(glm::vec2 uv, float scale, float offset) {
    float SCALE = scale;//800.0;
    uv = uv / SCALE;
    glm::vec2 coord = uv;
    glm::mat2 m = glm::mat2( 1.3, 2.2, -1.4, 1.2 );
    float h  = 0.5000 * SimplexNoise( coord );
    coord = m*coord;
    //  bug here h can be negative
    h += 0.2500*SimplexNoise( coord );
//    h = 0.5 * (h + 1.f);
//    std::cout<<(h>1)<<std::endl;
//    return h * (250 - 128) + 128;
    h = 0.5 * (h + 1.f);
    return h * 50 + offset;//128; //[78, 244]
}

float grassland(glm::vec2 uv, float scale, float offset) {
    float SCALE = scale;//50.0;
    uv = uv / SCALE;
    glm::vec2 coord = uv;
    glm::mat2 m = glm::mat2( 1.3, 2.2, -1.4, 1.2 );
    float h  = SimplexNoise( uv * 1.3f);
    coord = m*coord;
    //  bug here h can be negative
    h += 0.2500*SimplexNoise( coord );
    h = 0.5 * (h + 1.f);

    return h * 90 + offset;//135;
//    return glm::clamp(h, 0.f, 1.f) * (250 - 128) + 138;
}




float moisture(glm::vec2 uv){
    return SimplexNoise(uv);
}

float temperature(glm::vec2 uv){
    return PerlinNoise(uv);
}

float plain(glm::vec2 uv) {
    float SCALE = 50.0;
    uv = uv / SCALE;
    glm::vec2 coord = uv;
    return 1 - fbm(uv);
}



float caveCeil(glm::vec2 uv) {
    float SCALE = 50.0;
    uv = uv / SCALE;
    glm::vec2 coord = uv;
    glm::mat2 m = glm::mat2( 1.5, 1.8, -1.1, 2.2 );
    float h  = 0.5000*SimplexNoise( coord );
    coord = m*coord;
    h += 0.2500*SimplexNoise( coord );

    float h1 = glm::clamp(h, 0.f, 1.f) * 128.f;


    SCALE = 40.0;
    uv = uv / SCALE;
    m = glm::mat2( 2.9, -1.3, 1.4, -1.8 );
    h  = 0.5000*SimplexNoise( coord );
    coord = m*coord;
    h += 0.2500*SimplexNoise( coord );
    coord = m*coord;
    h += 0.12500*SimplexNoise( coord );

    float h2 = glm::clamp(h, 0.f, 1.f) * 32.f;
    if ( h1 > 0) {
       return h1 + 1;
    } else {
     return h2 + 1;
    }
}

float caveFloor(glm::vec2 uv) {
    float SCALE = 100.0;
    uv = uv / SCALE;
    glm::vec2 coord = uv;
    glm::mat2 m = glm::mat2( 2.5, 1.7, -0.9, 1.2 );
    float h  = 0.5000*SimplexNoise( coord );
    coord = m*coord;
    //  bug here h can be negative
    h += 0.2500*SimplexNoise( coord );

    return glm::clamp(h, 0.f, 1.f) * 32;
}

