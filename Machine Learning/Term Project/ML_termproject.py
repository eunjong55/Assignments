# References
# https://github.com/yunjey/pytorch-tutorial/blob/master/tutorials/01-basics/pytorch_basics/main.py
# http://pytorch.org/tutorials/beginner/data_loading_tutorial.html#dataset-class
# one-hot encoding
# https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.get_dummies.html
# https://teddylee777.github.io/scikit-learn/labelencoder-%EC%82%AC%EC%9A%A9%EB%B2%95

from torch.utils.data import Dataset, DataLoader
from torch import nn, from_numpy, optim
import numpy as np
from torch.utils.data.dataset import random_split
import pandas as pd
import torch
import torch.nn.functional as F
import matplotlib.pyplot as plt

class DiabetesDataset(Dataset):
    """ Diabetes dataset."""
    # Initialize your data, download, etc.
    def __init__(self):
        xy = np.loadtxt('./kr-vs-kp.data', delimiter=',', dtype=np.str)
        df = pd.read_csv('./kr-vs-kp.data', header=None)
        
        #one-hot encoding
        X = pd.get_dummies(df.iloc[:, :-1]) #
        X = X.to_numpy()
        X = X.astype(np.float32)
        
        #change won->1, nowin->0
        y = xy[:, 36:37]
        for i in range(len(y)):
            if y[i][0] == 'won':
                y[i][0] = 1
            else :
                y[i][0] = 0
        y = y.astype(np.float32)

        self.len = X.shape[0]
        self.x_data = from_numpy(X)
        self.y_data = from_numpy(y)
        
    def __getitem__(self, index):
        return self.x_data[index], self.y_data[index]

    def __len__(self):
        return self.len

dataset = DiabetesDataset()
batch_size = 32
#data split
lengths = [int(len(dataset)*0.8), len(dataset)-int(len(dataset)*0.8)]
train_set, test_set = random_split(dataset, lengths)

train_loader = DataLoader(dataset=train_set,
                          batch_size=batch_size,
                          shuffle=True,
                          )

test_loader = DataLoader(dataset=test_set,
                          batch_size=batch_size,
                          shuffle=True,
                          )      
class Model(nn.Module):
    def __init__(self):
        super(Model, self).__init__()
        self.l1 = nn.Linear(73, 480)
        self.l2 = nn.Linear(480, 280)
        self.l3 = nn.Linear(280, 80)
        self.l4 = nn.Linear(80, 40)
        self.l5 = nn.Linear(40, 1)

        self.relu = nn.ReLU()
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        x = F.relu(self.l1(x))
        x = F.relu(self.l2(x))
        x = F.relu(self.l3(x))
        x = F.relu(self.l4(x))
        return self.sigmoid(self.l5(x))

model = Model()
criterion = nn.BCELoss(reduction='mean')
optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.5)

# Training loop
model.train()
avg_loss_values = []
for epoch in range(30):
    train_loss = 0.0
    for i, data in enumerate(train_loader, 0):
        inputs, labels = data
        y_pred = model(inputs)
        loss = criterion(y_pred, labels)
        print(f'Epoch {epoch + 1} | Batch: {i+1} | Loss: {loss.item():.4f}')
        train_loss+= loss
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
    avg_loss_values.append(train_loss)
    
# Test part
model.eval()
test_loss = 0
correct = 0
for d, data in enumerate(test_loader, 0):
    inputs, labels = data
    output = model(inputs)
    for i in range(len(output)):
        if output[i] > 0.5:
            if labels[i] ==1:
                correct +=1
        else:
            if labels[i] == 0:
                correct +=1
    test_loss += criterion(output, labels).item()
test_loss /= len(test_loader.dataset)
print(f'===========================\nTest set: Average loss: {test_loss:.4f}, Accuracy: {correct}/{len(test_loader.dataset)} '
    f'({100. * correct / len(test_loader.dataset):.0f}%)')

#draw chart
plt.title('number of Epoches - loss graph')
plt.xlabel('number of Epochs')
plt.ylabel('loss')
plt.plot(np.array(avg_loss_values), 'r')