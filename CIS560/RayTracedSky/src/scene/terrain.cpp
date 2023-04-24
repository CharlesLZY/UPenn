#include <scene/terrain.h>

#include <scene/cube.h>

Terrain::Terrain() : dimensions(64, 256, 64)
{}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    return m_blocks[x][y][z];
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // TODO: Make this work with your new block storage!
    m_blocks[x][y][z] = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 127; y < 256; ++y)
            {
                if(y <= 128)
                {
                    if((x + z) % 2 == 0)
                    {
                        m_blocks[x][y][z] = STONE;
                    }
                    else
                    {
                        m_blocks[x][y][z] = DIRT;
                    }
                }
                else
                {
                    m_blocks[x][y][z] = EMPTY;
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        m_blocks[x][129][0] = GRASS;
        m_blocks[x][130][0] = GRASS;
        m_blocks[x][129][63] = GRASS;
        m_blocks[0][130][x] = GRASS;
    }
    for(int y = 129; y < 140; ++y)
    {
        m_blocks[32][y][32] = GRASS;
    }
}
