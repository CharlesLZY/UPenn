from copy import deepcopy

from Agent import Agent
from utils import Direction, Reward, ManhattanDistance, priorityQueue

'''
Without forward checking, search agent can not avoid suicuiding
'''

class GreedyAgent(Agent):
    def __init__(self, game):
        super(GreedyAgent, self).__init__(game)

    def _move(self):
        head_pos = self.game.head_pos
        food_pos = self.game.food_pos
        moves = self.game.valid_move()
        if moves:
            closest_dir = moves[0]
            min_dist = ManhattanDistance(food_pos, (head_pos[0]+closest_dir[0], head_pos[1]+closest_dir[1]))
            for choice in moves[1:]:
                dist = ManhattanDistance(food_pos, (head_pos[0]+choice[0], head_pos[1]+choice[1]))
                if dist < min_dist:
                    min_dist = dist
                    closest_dir = choice
            return closest_dir
        else: ### there is no valid move, go die
            return self.game.head_dir()


class AstarAgent(Agent):
    def __init__(self, game):
        self.game = game
        if game.W * game.H <= 10*10:
            self.game._setAgent(self)
            self.path = []
        else:
            print("The map is too large for search agent.")
            exit()
        
    ### Do not use whitespace
    def __valid_move(self, snake, food_pos):
        forward = (snake[-1][0] - snake[-2][0], snake[-1][1] - snake[-2][1])
        back = (-forward[0], -forward[1])
        directions = set([d.value for d in Direction])
        directions.remove(back)

        valid = []
        for d in directions:
            pos = (snake[-1][0] + d[0], snake[-1][1] + d[1])
            if pos == snake[0]: ### tail
                valid.append(d)
            elif pos not in snake and pos not in self.game.wall:
                valid.append(d)
        
        return valid

    def _move(self):
        if self.path: ### memoization
            return self.path.pop(0)
        else:
            head_pos = self.game.head_pos
            food_pos = self.game.food_pos

            snake = deepcopy(self.game.snake)
            
            fringe = priorityQueue() ### use priority queue for Astar successor function
            visited = set() ### visited list for graph search

            fringe.push((snake, []), ManhattanDistance(head_pos, food_pos))
            while not fringe.isEmpty():
                cur_snake, cur_path = fringe.pop()
                visited.add(tuple(cur_snake))
                if ManhattanDistance(cur_snake[-1], food_pos) == 0: ### the snake head is at the food_pos, we found the path
                    self.path = cur_path
                    return self.path.pop(0)

                g = len(cur_path)
                moves = self.__valid_move(cur_snake, food_pos)
                for d in moves:
                    new_snake = deepcopy(cur_snake)
                    new_head_pos = (cur_snake[-1][0] + d[0], cur_snake[-1][1] + d[1])
                    new_snake.popleft()
                    new_snake.append(new_head_pos)
                    new_state = tuple(new_snake)
                    if new_state in visited:
                        continue
                    h = ManhattanDistance(new_head_pos, food_pos)
                    fringe.push((new_snake, cur_path + [d]), g+1+h)


            return self.game.head_dir() ### not found any path to food