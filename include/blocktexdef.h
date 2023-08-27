#ifndef BLOCKTEXDEF_H
#define BLOCKTEXDEF_H

class BlockConfig {
public:
    int texUp    = 0;
    int texDown  = 0;
    int texLeft  = 0;
    int texRight = 0;
    int texBack  = 0;
    int texFront = 0;

    int collisionType = 1;
    int renderModel = 1;

    BlockConfig(int type, int model, int tup, int tdown, int tleft, int tright, int tback, int tfront)
        : texUp(tup), texDown(tdown), texLeft(tleft), texRight(tright), texBack(tback), texFront(tfront),
          collisionType(type), renderModel(model)
    {}

    BlockConfig(int type, int model, int tup)
        : texUp(tup), texDown(tup), texLeft(tup), texRight(tup), texBack(tup), texFront(tup),
          collisionType(type), renderModel(model)
    {}

    BlockConfig(int type, int model, int tup, int tdown, int tleft)
        : texUp(tup), texDown(tdown), texLeft(tleft), texRight(tleft), texBack(tleft), texFront(tleft),
          collisionType(type), renderModel(model)
    {}
};

const BlockConfig blockConfig[] = {
    {0, 1, 1}, // air
    {1, 1, 2}, // stone
    {1, 1, 1, 3, 4}, // grass block
    {1, 1, 3}, // dirt
    {1, 1, 17}, // cobblestone
    {1, 1, 5}, // planks
    {0, 4, 16}, // saplings
    {1, 1, 18}, // bedrock
    {0, 5, 1}, // water1
    {0, 5, 1}, // water2
    {0, 5, 1}, // lava1
    {0, 5, 1}, // lava2
    {1, 1, 19}, // sand
    {1, 1, 20}, // gravel
    {1, 1, 33}, // gold ore
    {1, 1, 34}, // iron ore
    {1, 1, 35}, // coal ore
    {1, 1, 21}, // logs
    {1, 3, 53}, // leaves
    {1, 1, 49}, // sponge
    {1, 1, 50}, // glass
    {1, 1, 161}, // lapis ore
    {1, 1, 145}, // lapis block
    {1, 1, 47}, // dispenser
    {1, 1, 1}, // sandstone
    {1, 1, 1}, // note blocks
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 5, 40},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
};

#endif
