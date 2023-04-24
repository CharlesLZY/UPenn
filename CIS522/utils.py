from heapq import *
from enum import Enum

class Direction(Enum):
    ### UP and Down can be considered as FORWARD
    UP    = ( 0,-1)
    DOWN  = ( 0, 1)
    LEFT  = (-1, 0)
    RIGHT = ( 1, 0)

class Reward(Enum): ### The reward will be return in SnakeGame._play()
    FOOD = 15
    CLOSER = 3 ### get closer to the food
    FURTHER = -1 ### get further from the food
    LIVE = 1
    DEATH = -50
    WIN = 100

class Value(Enum): ### The value to represent the map state
    WALL = 0
    EMPTY = 1
    SNAKE_BODY = 0
    SNAKE_HEAD = 0
    SNAKE_TAIL = 0
    FOOD = 5

def ManhattanDistance(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

class priorityQueue: # pop the entry with the lowest priority
    def  __init__(self):
        self.heap = []
        
    def push(self, item, priority):
        entry = (priority, item)
        heappush(self.heap, entry)
        
    def pop(self):
        _, item = heappop(self.heap)
        return item

    def isEmpty(self):
        return len(self.heap) == 0


def LongLiveSnake():
    print("\
              xxxxxxxxxxxx\n\
            xxx           xxxxxxxx\n\
           xx              xx-----xxxx\n\
          xx         xxx   x xx-------xxxx\n\
         xx         x| x   x   xx--------xx\n\
        xx         x|| x   x    xx---------xx\n\
        xx        x ||x    x      xx--------xx\n\
        xx         xxx    xx        x---------xx\n\
        xx               xx          x---------xx\n\
        xx              xx            x---------xx\n\
        xx             xx              x---------xx\n\
         xx           xx               x--------xx\n\
          xx       xxx                 x-------xx\n\
            xxxxxxx---xx               x-------x\n\
             xx  x------x             x------x      x\n\
             xx  x-------x           xx-----x      x-\n\
            xx    x------x          xx-----x      x-x\n\
           xx      x-----x         xx----xx      x-x\n\
          x x       x---x         xx---xx       x-x\n\
         x   x      x--x         xx---xx       x-x\n\
        x     x      xx         xx---xx       x-x\n\
       x      x      x         xx---xx       x--x\n\
                    x         xx---xx       x--xx\n\
                  xx         xx---xx       x--x x\n\
                 x          xx--xx        x--x  x\n\
               xx          xx--x        xx--x   x\n\
              xx          xx--xx      xx---x    x\n\
             x           xx-----xx   xx---x     x\n\
            x            xx------xxxx---xx     x\n\
            x             xx-----------x      x\n\
            x               xx-------xx      x\n\
             x                xxxxxxx       x\n\
              x                           xx\n\
               xx                       xx\n\
                 xx                   xx\n\
                   xxx             xxx\n\
                      xxxxxxxxxxxxx")


def whosBOSS():
    print("\
                                        ,==.\n\
                                        \\ o ',\n\
                        .--.             \\    \\.''..''..''.\n\
 _       _       _     '-._ \\            /    ;'..''..''..'\n\
|_|     |_|     |_|    .-'  /           /   .'         .''.    .''..''..''..''.\n\
                        '--'            \"==\"           '..'    '..''..''..''..'\n\
                                                       .''..''..''.\n\
                                                       '..''..''..'")