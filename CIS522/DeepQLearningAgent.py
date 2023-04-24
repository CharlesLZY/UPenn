import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F

import numpy as np 

import random

from Game import SnakeGame
from Agent import Agent
from utils import Direction

from AutoEncoder import Encoder


DEVICE = "cuda:0" if torch.cuda.is_available() else "cpu"
# print(DEVICE)

### the model predicts the q-value for each action given the state
class LinearModel(nn.Module):
    def __init__(self,input_size=6, hidden_size=128):
        super(LinearModel, self).__init__()
        self.linear1 = nn.Linear(input_size, hidden_size)
        self.linear2 = nn.Linear(hidden_size, 4) ### output_size = 4 because there are 4 actions
    
    def forward(self, x):
        x = torch.flatten(x) ### in case, we just pass the map to the linear model, it should flatten the 2d map first
        x = F.relu(self.linear1(x))
        x = self.linear2(x)
        return x

class CNNModel(nn.Module):
    def __init__(self, map_size, hidden_size=16):
        super(CNNModel, self).__init__()
        ### kernel size need to be tuned
        
        H, W = map_size[0] + 2, map_size[1] + 2 ### + 2 for the border wall
        self.conv1 = nn.Conv2d(in_channels = 1, out_channels =1, kernel_size = 5, padding = 2) ### in & out channels are 1, stride = 1
        self.conv2 = nn.Conv2d(in_channels = 1, out_channels =1, kernel_size = 3)
        ### size after conv1: (H - 5 + 2*2）// 1 + 1 = H
        ### size after conv2: (H - 3) // 1 + 1
        ### if we use CNN, we have to compute the final size after convolution to use fully connected layer
        self.fc1 = nn.Linear((H-3 + 1) * (W-3 + 1), hidden_size)
        self.fc2 = nn.Linear(hidden_size, 4) ### output_size = 4 because there are 4 actions
    
    def forward(self, x):
        x = self.conv1(x)
        x = self.conv2(x)
        x = torch.flatten(x)
        x = F.relu(self.fc1(x))
        x = self.fc2(x)
        return x





class DeepQLearningAgent(Agent):
    def __init__(self, game, model_type, 
                 input_size=6, 
                 map_size=None, 
                 encoder=None, 
                 pretrained_model=None):
        super(DeepQLearningAgent, self).__init__(game)

        self.model_type = model_type ### set the model type

        if model_type == "linear":
            self.model = LinearModel(input_size=input_size).to(DEVICE)

        elif model_type == "cnn":
            if map_size is None:
                print("Missing map_size!")
                exit()
            self.model = CNNModel(map_size).to(DEVICE)
        
        elif model_type == "encoder":
            if encoder is None:
                print("Missing encoder!")
                exit()
            self.encoder = encoder
            self.model = LinearModel(input_size=input_size).to(DEVICE)
        
        if pretrained_model:
            self._loadModel(pretrained_model)

    def __cur_state(self):
        if self.model_type == "linear":
            return self.game.naive_state().to(DEVICE)

        elif self.model_type == "cnn":
            return self.game.map_state().to(DEVICE)

        elif self.model_type == "encoder":
            return self.game.encoder_state(self.encoder).to(DEVICE)
    
    
        
    def _move(self):
        directions = [d.value for d in Direction]
        state = self.__cur_state()
        prediction = self.model(state)
        move = directions[torch.argmax(prediction).item()] ### choose the move with the highest score
        return move

    def _saveModel(self, filename="model/linear.pth"):
        torch.save(self.model.state_dict(), filename)

    def _loadModel(self, filename):
        self.model.load_state_dict(torch.load(filename, map_location=torch.device(DEVICE)))

    def __train_step(self, optimizer, criterion, discount, state, action, reward, new_state):
        directions = [d.value for d in Direction]
        action = directions.index(action) ### 0 1 2 3
        pred = self.model(state)
        target = pred.clone()

        sample = 0
        if new_state == None: # terminal state
            sample = reward
        else:
            ### only the current best action's Q-value will be updated according Bellman-Equation
            sample = reward + discount * torch.max(self.model(new_state))
        target[action] = sample

        optimizer.zero_grad()
        loss = criterion(target, pred)
        loss.backward()
        optimizer.step()


    def train(self, lr=0.01, discount=0.8, epsilon=1.0, ed=0.01, n_epoch=200, filename="model/new_model.pth"):
        '''
        lr: learning rate
        discount: discount to make the Q-value converge
        epsilon: possibility to make the random move to explore the state
        ed: epsilon decay rate of epsilon after each round training
        n_epoch: total training round
        '''
        optimizer = optim.Adam(self.model.parameters(), lr=lr)
        ### the action is based on the Q-values predicted by the model
        criterion = nn.MSELoss()

        for epoch in range(n_epoch):
            t = 0 ### to avoid infinite loop like the snake keep chasing its tail
            while t < 200:
                action = self._move()
                if random.uniform(0,1) < epsilon: ### force to make random move to explore state space
                    action = random.choice(list(Direction)).value

                state = self.__cur_state()
                reward, dead = self.game._play(move=action) ### reward and whether the game has ended
                new_state = self.__cur_state() if not dead else None ### s'

                self.__train_step(optimizer, criterion, discount, state, action, reward, new_state)

                if dead and epsilon > 0:
                    epsilon -= ed ### decay the epsilon
                    break

                t += 1
            if epoch % 10 == 1 and self.game.record:
                print(f"Current epoch: {epoch} Highest Score: {max(self.game.record)}")
        
        self._saveModel(filename=filename)


if __name__ == "__main__":
    '''
    It is highly recommended that train on the small-sized map first and then go to larger map to train the snake
    When train in the small-sized map, remember to set the t in train() to a small number (e.g. 100)
    During the further training round, remember to set the epsilon in train() to 0 
    '''
    # game = SnakeGame(W=10, H=10, SPEED=50)
    # agent = DeepQLearningAgent(game, "linear")
    # agent = DeepQLearningAgent(game, "linear", pretrained_model='model/linear-cpu.pth')
    

    # agent = DeepQLearningAgent(game, "cnn", map_size=(10,10))

    
    # agent.train(epsilon=1.0, ed=0.005, n_epoch=200, filename='model/test.pth')

    # while True:
    #     game._play()

    ### Augmented with auto-encoder
    ### Pipeline: 
    ### 1. set up the Value which represents different types of blocks (defined in utils.py)
    ### 2. train a auto-encoder (which is defined in AutoEncoder.py)
    ### 3. load the trained-parameter to an Encoder model (which is defined in AutoEncoder.py)
    ### 4. train a deep qlearning model use the output of the Encoder model as input feature
    ###    just call game.encoder_state()

    game = SnakeGame(W=16, H=16, SPEED=50)
    encoder = Encoder(W=16, H=16, feature_size=8)
    encoder.encoder.load_state_dict(torch.load("model/autoencoder-cpu.pth"))
    agent = DeepQLearningAgent(game, "encoder", input_size=8, encoder=encoder)
    agent.train(epsilon=1.0, ed=0.005, n_epoch=2, filename='model/test.pth')
