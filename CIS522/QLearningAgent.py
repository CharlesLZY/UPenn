import pickle
import random

from Game import SnakeGame
from Agent import Agent
from utils import Direction

'''
Naive Q state : 
(surrounding obstacle, food direction)
I used 4 bits to indicate whether there are obstacles surrounding the snake head
(0/1, 0/1, 0/1, 0/1): (up, down, left, right)
I used 2 bits to indicate the food direction:
(-1, 1) ( 0, 1) ( 1, 1)
(-1, 0)   head  ( 1, 0)
(-1,-1) (0, -1) ( 1,-1)
There are 2*2*2*2*8 = 128 states in total.
There are 4 possible actions UP, DOWN, LEFT, RIGHT in each state.
Therefore, the Q table size will be 128 * 4 = 512.
'''

class QLearningAgent(Agent):
    def __init__(self, game, pretrained_model=None):
        super(QLearningAgent, self).__init__(game)
        if pretrained_model:
            self._loadModel(pretrained_model)
        else:
            self.__generateQTable()

    def __generateQTable(self):
        self.QTable = {} ### {(state, action): Q-value}
        food_directions = [(-1,-1), (0,-1), (1,-1), (-1,0), (1,0), (-1,1), (0,1), (1,1)]
        for action in Direction:
            for dx, dy in food_directions:
                for up in range(2):
                    for down in range(2):
                        for left in range(2):
                            for right in range(2):
                                self.QTable[((up, down, left, right, dx, dy), action.value)] = 0
    
    def __cur_state(self):
        head_pos = self.game.head_pos
        food_pos = self.game.food_pos
        whitespace = self.game.whitespace
        assert food_pos is not None ### if there is no food on the board, the snake must have filled the board

        ### 6 bits state (up, down, left, right, food_dirX, food_dirY)
        state = [None]*6
        
        up_pos = (head_pos[0], head_pos[1]-1)
        if up_pos in whitespace or up_pos == food_pos: ### empty space at up pos
            state[0] = 0
        else: ### there is obstacle at up pos
            state[0] = 1

        down_pos = (head_pos[0], head_pos[1]+1)
        if down_pos in whitespace or down_pos == food_pos: ### empty space at down pos
            state[1] = 0
        else: ### there is obstacle at down pos
            state[1] = 1
        
        left_pos = (head_pos[0]-1, head_pos[1])
        if left_pos in whitespace or left_pos == food_pos: ### empty space at left pos
            state[2] = 0
        else: ### there is obstacle at left pos
            state[2] = 1

        right_pos = (head_pos[0]+1, head_pos[1])
        if right_pos in whitespace or right_pos == food_pos: ### empty space at right pos
            state[3] = 0
        else: ### there is obstacle at right pos
            state[3] = 1
        
        ### food direction x
        if food_pos[0] > head_pos[0]:
            state[4] = 1
        elif food_pos[0] < head_pos[0]:
            state[4] = -1
        else:
            state[4] = 0
        ### food direction y
        if food_pos[1] > head_pos[1]:
            state[5] = 1
        elif food_pos[1] < head_pos[1]:
            state[5] = -1
        else:
            state[5] = 0
        
        return tuple(state)
    
    def __updateQValue(self, reward, Q_state, new_state, lr, discount):
        '''
        Q-Learning
        sample = R(s, a, s') + gamma * max_a'(Q(s', a'))
        Q(s,a) <- (1 - alpha) * Q(s,a) + alpha * sample
        '''
        sample = 0
        actions = [d.value for d in Direction]
        if new_state == None: # terminal state
            sample = reward
        else:
            sample = reward + discount * max(map(lambda a: self.QTable[(new_state, a)], actions))
        self.QTable[Q_state] = (1 - lr) * self.QTable[Q_state] + lr * sample

    def _saveModel(self, filename="model/ql.pkl"):
        f = open(filename, 'wb')
        f.write(pickle.dumps(self.QTable))
        f.close()

    def _loadModel(self, filename):
        with open(filename, 'rb') as f:
            self.QTable = pickle.loads(f.read())

    def _move(self):
        state = self.__cur_state()
        Q_vals = {}
        for action in Direction:
            Q_vals[action.value] = self.QTable[(state, action.value)]
        return max(Q_vals, key=Q_vals.get) ### return the action which has the max Q-value in current state

    def train(self, alpha=0.9, discount=0.8, epsilon=1.0, ed=0.01, n_epoch=200,  filename="model/ql.pkl"):
        '''
        alpha: learning rate
        discount: discount to make the Q-value converge
        epsilon: possibility to make the random move to explore the state
        ed: epsilon decay rate of epsilon after each round training
        n_epoch: total training round
        '''
        for epoch in range(n_epoch):
            t = 0 ### to avoid infinite loop like the snake keep chasing its tail
            while t < 1000:
                action = self._move()
                if random.uniform(0,1) < epsilon: ### force to make random move to explore state space
                    action = random.choice(list(Direction)).value

                Q_state = (self.__cur_state(), action) ### Q(s,a)
                reward, dead = self.game._play(move=action) ### reward and whether the game has ended
                new_state = self.__cur_state() if not dead else None ### s'
                self.__updateQValue(reward, Q_state, new_state, alpha, discount)

                if dead:
                    epsilon -= ed ### decay the epsilon
                    break

                t += 1
            
        self._saveModel(filename = filename)

if __name__ == "__main__":
    game = SnakeGame(W=5, H=5)
    agent = QLearningAgent(game)
    agent.train(epsilon=1.0, ed=0.005, n_epoch=200, filename='model/test.pth')