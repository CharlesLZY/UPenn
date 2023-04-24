from Agent import Agent
from utils import Direction

### the snake will move in the fixed pattern to win the game
class BoringAgent(Agent):
    def __init__(self, game):
        super(BoringAgent, self).__init__(game)
        self.flag = 1 ### for odd-sized case, flag to switch the move pattern

    def _move(self):
        x, y = self.game.head_pos
        W, H = self.game.W, self.game.H

        if W % 2 == 0:
            '''
            0 <————————— 
            1 |  _   _  |
            2 | | | | | |
            3 | | | | | |
            4  —>  —>  —>
              0 1 2 3 4 5
            '''
            
            if y == 0: ### the top row
                if x == 0: ### the left-top corner
                    return Direction.DOWN.value
                else:
                    return Direction.LEFT.value

            elif y == H - 1: ### the bottom row
                if x % 2 == 0:
                    return Direction.RIGHT.value
                else:
                    return Direction.UP.value
            
            elif y == 1 and x != W-1: ### the second row
                if x % 2 == 1:
                    return Direction.RIGHT.value
                else:
                    return Direction.DOWN.value
            
            else:
                if x % 2 == 0:
                    return Direction.DOWN.value
                else:
                    return Direction.UP.value


        elif H % 2 == 0:
            ''' 
            0 <————————— 
            1 |  _______|
            2 | |_______
            3 |  _______|
            4 | |_______ 
            5 |_________|
              0 1 2 3 4 5
            '''
            if x == 0: ### the left col
                if y == H-1: ### the left-bottom corner
                    return Direction.RIGHT.value
                else:
                    return Direction.DOWN.value

            elif x == W - 1: ### the right col
                if y % 2 == 0:
                    return Direction.LEFT.value
                else:
                    return Direction.UP.value
            
            elif x == 1 and (y != 0 and y != H-1): ### the second col
                if y % 2 == 1:
                    return Direction.RIGHT.value
                else:
                    return Direction.UP.value
            
            else:
                if y % 2 == 0:
                    return Direction.LEFT.value
                else:
                    return Direction.RIGHT.value

        else: ### odd-sized case
            ''' 
         0   ___________              0   ________
         1  |   __    __|             1  |   __   |__
         2  |  |  |  |__      ——>     2  |  |  |   __|
         3  |  |  |   __|             3  |  |  |  |__
         4  |__|  |__|                4  |__|  |_____|
            0  1  2  3  4                0  1  2  3  4
             miss (4, 4)                  miss (4, 0)

            ''' 
            if x == W-2 and y == H-1:
                self.flag = -self.flag ### switch the move pattern

            if x < W-2:
                if y == 0: ### the top row
                    if x == 0: ### the left-top corner
                        return Direction.DOWN.value
                    else:
                        return Direction.LEFT.value
                
                elif y == H-1: ### the bottom row
                    if x % 2 == 0:
                        return Direction.RIGHT.value
                    else:
                        return Direction.UP.value
                elif y == 1: ### the second row
                    if x % 2 == 1:
                        return Direction.RIGHT.value
                    else:
                        return Direction.DOWN.value
                else:
                    if x % 2 == 0:
                        return Direction.DOWN.value
                    else:
                        return Direction.UP.value
            
            else: ### the last two cols
                if self.flag == 1:
                    if x == W-2:
                        if y == 0:
                            return Direction.LEFT.value
                        if y % 2 == 0:
                            return Direction.UP.value
                        else:
                            return Direction.RIGHT.value
                    else: ### x == W-1
                        if y % 2 == 1:
                            return Direction.UP.value
                        else:
                            return Direction.LEFT.value
                else:
                    if x == W-2:
                        if y == 0:
                            return Direction.LEFT.value
                        if y % 2 == 1:
                            return Direction.UP.value
                        else:
                            return Direction.RIGHT.value
                    else: ### x == W-1
                        if y % 2 == 0:
                            return Direction.UP.value
                        else:
                            return Direction.LEFT.value
            