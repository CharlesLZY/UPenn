import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader

import numpy as np

from collections import deque
import random
import pickle

from utils import Direction, Value

DEVICE = "cuda:0" if torch.cuda.is_available() else "cpu"

'''
IMPORTANT!!!
For auto-encoder, we need to use fixed map size 
and values which represent different type of block.
The Value is defined in utils.py
If you modify the map-size or Value, you have to 
re-train an auto-encoder.
'''

'''
Currently, we set the map size as W = 16 and H = 16.
Please check the Value in utils.py

Encoder.encoder should have the same structure as AutoEncoder.encoder
Encoder will use the trained parameter from AutoEncoder
'''
class Encoder(nn.Module):
    def __init__(self, W=16, H=16, feature_size=8):
        super(Encoder, self).__init__()

        self.encoder = nn.Sequential(
            torch.nn.Linear(W * H, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 32),
            torch.nn.ReLU(),
            torch.nn.Linear(32, feature_size),
        )


    def forward(self, x):
        encoded = self.encoder(x)
        return encoded

class AutoEncoder(nn.Module):
    def __init__(self, W=16, H=16, feature_size=8):
        super(AutoEncoder, self).__init__()

        self.encoder = nn.Sequential(
            torch.nn.Linear(W * H, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 32),
            torch.nn.ReLU(),
            torch.nn.Linear(32, feature_size),
        )

        self.decoder = nn.Sequential(
            torch.nn.Linear(feature_size, 32),
            torch.nn.ReLU(),
            torch.nn.Linear(32, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, W*H),
        )


    def forward(self, x):
        encoded = self.encoder(x)
        decoded = self.decoder(encoded)
        return decoded


### Simplified snake game without pygame stuff, only considering the map state
class GameState:
    def __init__(self, W=16, H=16):
        self.W = W
        self.H = H
        self._setWall()
        self._reset()
    
    def _setWall(self):
        self.wall = set()
    
    def _reset(self):
        self.whitespace = set([(x,y) for x in range(self.W) for y in range(self.H)]) ### for fast generating new food 
        ### remove wall
        for wall in self.wall:
            self.whitespace.remove(wall)
        ### initialize food
        self.food_pos = random.choice(list(self.whitespace))
        self.whitespace.remove(self.food_pos)
        ### initialize head
        head_pos = random.choice(list(self.whitespace))
        self.snake = deque([head_pos]) ### snake[-1] is head and snake[0] is tail
        self.whitespace.remove(self.snake[0])

    '''
    The state will be 2D array which looks like:
    1 1 1 1 1
    1 5 1 0 1 
    1 1 0 0 1
    1 0 0 1 1
    1 0 1 1 1 
    where food is located in (1,1) which is five and the snake is represented by 0, 1 means EMPTY
    '''
    def cur_state(self): ### same as the map_state in SnakeGame class
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
        return state

        # state = torch.tensor(state, dtype=torch.float).to(DEVICE)
        # return state.reshape(1,1, H, W)

    ### extend the snake body randomly until it grows to the length
    def grow(self, length):
        directions = set([d.value for d in Direction])
        for i in range(length-1):
            valid_pos = []
            cur_tail = self.snake[0]
            for d in directions:
                pos = (cur_tail[0] + d[0], cur_tail[1] + d[1])
                if pos in self.whitespace:
                    valid_pos.append(pos)
            if len(valid_pos) == 0: ### the snake can not grow any more
                break
            
            new_body = random.choice(valid_pos)
            self.snake.appendleft(new_body)
            self.whitespace.remove(new_body)

### refer to https://pytorch.org/tutorials/beginner/data_loading_tutorial.html
class MapStateDataset(Dataset):
    def __init__(self, filename, flatten=True):
        self.flatten = flatten
        f = open(filename, 'rb')
        self.map_states = pickle.loads(f.read())
        f.close()
    
    def __len__(self):
        return len(self.map_states)
    
    def __getitem__(self, idx):
        if self.flatten: ### for fully connected encoder
            return torch.from_numpy(self.map_states[idx]).flatten().float()
        else: ### return 2d array (for CNN encoder (not implemented))
            return torch.from_numpy(np.expand_dims(self.map_states[idx], axis=0)).float() ### shape: (batch_size, 1, W, H)



### generate map states with different snakes 
def generateMapState(W=16, H=16, N=10000, filename="data/empty_map.pkl"):
    game = GameState(W=W, H=H)
    states = []
    for i in range(N): ### generate N scenes
        game._reset()
        length = random.randint(1, W*H-2)
        game.grow(length)
        states.append(game.cur_state())

    f = open(filename, 'wb')
    f.write(pickle.dumps(states))
    f.close()

def train(n_epoch=10, batch_size=4, W=16, H=16, feature_size=8, dataset="data/empty_map.pkl", save_path="model/autoencoder-cpu.pth"):
    dataset = MapStateDataset(dataset)
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    model = AutoEncoder(W=W, H=H, feature_size=feature_size)
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr = 1e-1, weight_decay = 1e-8)
    for epoch in range(n_epoch):
        for data in dataloader:
            reconstructed = model(data)
            loss = criterion(reconstructed, data)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
        if epoch % 10 == 0:
            print(f"epoch: {epoch} loss: {loss.item()}")

    param = model.encoder.state_dict()
    torch.save(param, save_path)



if __name__ == "__main__":
    # model = AutoEncoder().to(DEVICE)
    # encoder = Encoder().to(DEVICE)

    # param = model.encoder.state_dict()
    # torch.save(param, "test.pth")
    # print(param)

    # encoder.encoder.load_state_dict(torch.load("test.pth"))
    # print(encoder.state_dict())

    # generateMapState()
    
    # dataset = MapStateDataset("data/empty_map.pkl")
    # dataloader = DataLoader(dataset, batch_size=4, shuffle=True)
    # for sample in dataloader:
    #     print(sample.size())
    #     break

    train(n_epoch=2)