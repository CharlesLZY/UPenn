import pygame
import torch
import numpy as np

from enum import Enum
from collections import deque
import random

from AutoEncoder import Encoder

from utils import Direction, Reward, Value, ManhattanDistance

DEVICE = "cuda:0" if torch.cuda.is_available() else "cpu"

class Color(Enum):
    WHITE = (255,255,255)
    GREY1 = (100,100,100)
    GREY2 = (200,200,200)
    BLACK = (0,0,0)
    RED   = (255,0,0)
    GREEN = (0,255,0)
    BLUE   = (0,0,255)

class SnakeGame:
    def __init__(self, W=16, H=16, BLOCK_SIZE=20, SPEED=50, VERBOSE=False, SEED=None):
        ### set random seed
        random.seed(SEED)

        ### Set Game Parameter
        self.W = W
        self.H = H
        self.Width = (self.W) * BLOCK_SIZE ### window width
        self.Height = (self.H) * BLOCK_SIZE ### window height 
        self.BLOCK_SIZE = BLOCK_SIZE if BLOCK_SIZE > 20 else 20 ### block size for display
        self.SPEED = SPEED
        self.VERBOSE = VERBOSE ### whether to print game information

        ### The game agent
        self.agent = None
        self.round = 0 ### current round
        self.record = []

        ### Initial GUI
        pygame.init()
        self.display = pygame.display.set_mode((self.Width, self.Height))
        pygame.display.set_caption('Snake')
        self.clock = pygame.time.Clock()

        ### Initial Game State
        self._setWall()
        self._restart()
        self._renderGUI()
    
    ### for the scalability to implement maze
    def _setWall(self):
        self.wall = set()

    
    ### reset the game state to initial state
    def _restart(self):
        self.round += 1 ### a new round start
        self.whitespace = set([(x,y) for x in range(self.W) for y in range(self.H)]) ### for fast generating new food 

        for wall in self.wall:
            self.whitespace.remove(wall)

        self.head_pos = (self.W // 2, self.H // 2) ### the initial postion of head
        ### snake[-1] is head and snake[0] is tail
        self.snake = deque([(self.head_pos[0], self.head_pos[1] + 1), self.head_pos]) ### for fast updating snake
        self.whitespace.remove(self.snake[0])
        self.whitespace.remove(self.snake[1])
        self.score = 0 ### current score
        self.current_step = 0 ### how many steps the snake has moved
        
        self._placeFood() ### place food


    def _setAgent(self, agent):
        self.agent = agent ### set player

    def _placeFood(self):
        if len(self.whitespace) > 0:
            # self.food_pos = self.whitespace.pop() ### set food position by randomly choosing a whitespace
            self.food_pos = random.choice(list(self.whitespace))
            self.whitespace.remove(self.food_pos)

        else: ### there is no space to place food
            self.food_pos = None

    def _nextMove(self):
        self.current_step += 1

        if self.agent:
            for event in pygame.event.get():
                if(event.type == pygame.QUIT):
                    pygame.quit()
                    quit()
            
            return self.agent._move()

        else: ### default: manipulate the snake by keyboard
            for event in pygame.event.get():
                if(event.type == pygame.QUIT):
                    pygame.quit()
                    quit()
                if(event.type == pygame.KEYDOWN):
                    if(event.key == pygame.K_LEFT):
                        return Direction.LEFT.value
                    elif(event.key == pygame.K_RIGHT):
                        return Direction.RIGHT.value
                    elif(event.key == pygame.K_UP):
                        return Direction.UP.value
                    elif(event.key == pygame.K_DOWN):
                        return Direction.DOWN.value
            
            return self.head_dir() ### default: move forward

    ### play the game
    def _play(self, move=None, GUI=True): ### we can specify the move instead of let the agent decide the next move
        ### Reward is defined in utils.py
        reward = Reward.LIVE.value
        dead = False

        if self.food_pos:
            prev_distance = ManhattanDistance(self.head_pos, self.food_pos) ### previous distance from snake head to food

            ### if the next move has been specified, we will ignore the agent's decision
            dx, dy = move if move else self._nextMove() ### direction
            new_head_pos = (self.head_pos[0] + dx, self.head_pos[1] + dy)
            
            self.head_pos = new_head_pos ### update head pos
            self.snake.append(new_head_pos) ### add new head to the snake

            
            if new_head_pos != self.food_pos:
                self.whitespace.add(self.snake.popleft()) ### add the tail to the whitespace and remove the tail from the snake

                if self._isCollision(): ### snake hit the wall or ate itself, it dead
                    reward = Reward.DEATH.value 
                    dead = True

                    if self.VERBOSE:
                        print(f"Round: {self.round} Score: {self.score} Steps: {self.current_step}")
                    self.record.append((self.score, self.current_step))
                    self._restart()
                else:
                    cur_distance = ManhattanDistance(self.head_pos, self.food_pos) ### current distance from snake head to food
                    if cur_distance > prev_distance: ### get further from the food
                        reward = Reward.FURTHER.value
                    elif cur_distance < prev_distance: ### get closer to the food
                        reward = Reward.CLOSER.value

            else: ### the snake ate the food
                ### in this case, there must not be collision
                self.score += 1
                self._placeFood()
                
                reward = Reward.FOOD.value 
                dead = True
            
            self.whitespace.discard(new_head_pos) ### remove the new head from the whitespace

        else: ### the snake filled the whole board
            reward = Reward.WIN.value 
            dead = True

            if self.VERBOSE:
                print(f"Round: {self.round} Score: {self.score} Steps: {self.current_step}")
            self.record.append((self.score, self.current_step))
            self._restart()
        

        if GUI:
            self._renderGUI() ### render current state

        return reward, dead


    def _isCollision(self):
        if self.head_pos in self.whitespace:
            return False
        else:
            return True

    ### render the GUI
    def _renderGUI(self):
        self.display.fill(Color.WHITE.value)
        BLOCK_SIZE = self.BLOCK_SIZE
        BORDER = 4

        ### draw the wall
        for x,y in self.wall:
            pygame.draw.rect(self.display, Color.BLACK.value, pygame.Rect((x) * BLOCK_SIZE, (y) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))

        ### draw the snake
        for x,y in self.snake:
            pygame.draw.rect(self.display, Color.GREY1.value, pygame.Rect((x) * BLOCK_SIZE, (y) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))
            pygame.draw.rect(self.display, Color.GREY2.value, pygame.Rect((x) * BLOCK_SIZE + BORDER, (y) * BLOCK_SIZE + BORDER, BLOCK_SIZE - 2 * BORDER, BLOCK_SIZE - 2 * BORDER))
        
        ### color the head
        pygame.draw.rect(self.display, Color.BLUE.value, pygame.Rect((self.snake[-1][0]) * BLOCK_SIZE, (self.snake[-1][1]) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))
        ### color the tail
        pygame.draw.rect(self.display, Color.RED.value, pygame.Rect((self.snake[0][0]) * BLOCK_SIZE, (self.snake[0][1]) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))

        if self.food_pos: ### if there is food, draw the food
            pygame.draw.rect(self.display, Color.GREEN.value, pygame.Rect((self.food_pos[0]) * BLOCK_SIZE, (self.food_pos[1]) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE))
        
        pygame.display.update()
        self.clock.tick(self.SPEED)

    ### forward direction
    def head_dir(self): 
        ### body -> head = head - body : BH = OH - OB
        return (self.snake[-1][0] - self.snake[-2][0], self.snake[-1][1] - self.snake[-2][1])
    
    def tail_dir(self):
        ### tail -> body = body - tail : TB = OB - OT
        return (self.snake[1][0] - self.snake[0][0], self.snake[1][1] - self.snake[0][1])

    ### current valid moves
    def valid_move(self):
        forward = self.head_dir()
        back = (-forward[0], -forward[1])

        directions = set([d.value for d in Direction])
        directions.remove(back)

        valid = []
        for d in directions:
            pos = (self.head_pos[0] + d[0], self.head_pos[1] + d[1])
            if pos == self.snake[0] or pos == self.food_pos: ### tail or food
                valid.append(d)
            elif pos in self.whitespace:
                valid.append(d)
        
        return valid
        
    def naive_state(self):
        '''
        The same state as Q learning agent used.
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

        head_pos = self.head_pos
        food_pos = self.food_pos
        whitespace = self.whitespace
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
        
        return torch.tensor(state, dtype=torch.float).to(DEVICE)


    ### Theoretically, CNN may not work well because of its strong prior assumption:
    ### the image pixels have strong relationship with its neighbours
    ### local features may not be useful in snake game
    def surrounding_state(self, r=3): 
        '''
        the surrounding state is defined as the area of which the snake head is the center
                  |
                  r
                  |
        <-- r -- head -- r -->
                  |
                  r
                  |        
        '''
        WALL = Value.WALL.value 
        EMPTY = Value.EMPTY.value 
        SNAKE_BODY = Value.SNAKE_BODY.value 
        SNAKE_HEAD = Value.SNAKE_HEAD.value 
        SNAKE_TAIL = Value.SNAKE_TAIL.value 
        FOOD = Value.FOOD.value 

        W = self.W
        H = self.H
        empty_space = list(self.whitespace)
        food_pos = self.food_pos
        head_pos = self.snake[-1]
        tail_pos = self.snake[0] 
        body = list(self.snake)[1: len(self.snake)-1]
        wall = list(self.wall)
        
        state = np.zeros((2*r+1, 2*r+1))
        ### (offset_x, offset_y) is the coordinate of the left-up corner of the surrounding area
        offset_x, offset_y = head_pos[0] - r, head_pos[1] - r 

        ### set the position outside the map as WALL
        for x in range(2*r+1):
            for y in range(2*r+1):
                if x + offset_x < 0 or y + offset_y < 0 or x + offset_x >= W or y + offset_y >= H:
                    state[y][x] = WALL

        ### set wall
        for px, py in wall:
            x, y = px - offset_x, py - offset_y
            if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
                continue
            else:
                state[y][x] = WALL    

        ### set empty space
        for px, py in empty_space:
            x, y = px - offset_x, py - offset_y
            if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
                continue
            else:
                state[y][x] = EMPTY   
        
        ### set snake body
        for px, py in body:
            x, y = px - offset_x, py - offset_y
            if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
                continue
            else:
                state[y][x] = SNAKE_BODY
        
        ### set snake tail
        px, py = tail_pos
        x, y = px - offset_x, py - offset_y
        if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
            pass
        else:
            state[y][x] = SNAKE_TAIL

        ### set snake head
        px, py = head_pos
        x, y = px - offset_x, py - offset_y
        if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
            pass
        else:
            state[y][x] = SNAKE_HEAD

        ### set food
        px, py = food_pos
        x, y = px - offset_x, py - offset_y
        if x < 0 or y < 0 or x >= 2*r+1 or y >= 2*r+1:
            pass
        else:
            state[y][x] = FOOD
        
        print(state)
        state = torch.tensor(state, dtype=torch.float).to(DEVICE)
        return state.reshape(1,1, 2*r+1, 2*r+1)

    def map_state(self, encoder=False):
        ### Value is defined in utils.py
        WALL = Value.WALL.value 
        EMPTY = Value.EMPTY.value 
        SNAKE_BODY = Value.SNAKE_BODY.value 
        SNAKE_HEAD = Value.SNAKE_HEAD.value 
        SNAKE_TAIL = Value.SNAKE_TAIL.value 
        FOOD = Value.FOOD.value 

        W = self.W
        H = self.H
        empty_space = list(self.whitespace)
        food_pos = self.food_pos
        head_pos = self.snake[-1]
        tail_pos = self.snake[0] 
        body = list(self.snake)[1: len(self.snake)-1]
        wall = list(self.wall)
        
        
        state = np.zeros((H, W))

        ### set wall
        for x, y in wall:
            state[y][x] = WALL     

        ### set empty space
        for x, y in empty_space:
            state[y][x] = EMPTY   
        
        ### set snake body
        for x, y in body:
            state[y][x] = SNAKE_BODY
        
        ### set snake tail
        x, y = tail_pos
        state[y][x] = SNAKE_TAIL

        ### set snake head
        x, y = head_pos
        state[y][x] = SNAKE_HEAD

        ### set food
        x, y = food_pos
        state[y][x] = FOOD    

 
        
        # print(state)
        if encoder:
            ### return which shape of tensor depends on which kind of structure does the encode use 
            return torch.from_numpy(state).flatten().float()
            ### for CNN encoder (not implemented)
            ### return torch.from_numpy(np.expand_dims(self.map_states[idx], axis=0)).float() ### shape: (batch_size, 1, W, H)

        else:
            state = torch.tensor(state, dtype=torch.float).to(DEVICE)
            return state.reshape(1,1, H, W)


    '''
    IMPORTANT!!!
    You must make sure the model you trained used the same map size as current game does.
    '''
    def encoder_state(self, model):
        state = self.map_state(encoder=True)
        model.eval()
        x = model(state)
        return x.detach()