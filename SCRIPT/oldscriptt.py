 #!-*- coding:utf-8 -*-
from multiprocessing import Pool
import MultiNEAT as NEAT
import subprocess
import time
import json
import os
import random
pop = NEAT.Population("ans10.txt")
pop2 = NEAT.Population("ans143.txt")
pop3 = NEAT.Population("ans183.txt")
pop4 = NEAT.Population("ans231.txt")
pop5 = NEAT.Population("ans.txt")
#a	messages::PressLeft()	Move left
#d	messages::PressRight()	Move right
#s	messages::PressDown()	Move down
#f	messages::PressA()	Press the A button (the remote emulator mapped that key to jump)
#j	messages::PressB()	Press the B button (the remote emulator doesn't use that key)
#w	messages::PressUp()	Move up
#k	messages::PressStart()	Press the Start button
#r	messages::GameReset()	Reset the game start from the beginning
#i	messages::GameImage()	Get an image of the game state as jpeg
#c	messages::ConfigFrame()	Set the number of frames before the next interaction to 30
#g	messages::GameInfo()	Get all game state informations including the tiles


trans = ('a', 'd', 's', 'f', 'j', 'w', 'k', 'r', 'i', 'c', 'g')
keyvalve = ('-6', '-5', '-4', '-3', '-2', '-1', '0', '1', '2', '3', '4','5', '6')


def evaluate(genome):

    #port = random.randint(20001,65535)
    # this creates a neural network (phenotype) from the genome

    net = NEAT.NeuralNetwork()
    genome.BuildPhenotype(net)

    port = random.randint(65000,65535)

    # let's input just one pattern to the net, activate it once and get the output
    #os.system("rm ./result"+str(port)+".txt")
    fdout = open("./result"+str(port)+".txt", "wb")
    fdin = open("./result"+str(port)+".txt", "r")
    null = open("/dev/null","w")

    #os.system('pkill nes')

    os.system('./gen.sh '+str(port))
    fce = subprocess.Popen("fceux --loadlua "+str(port)+".lua ../Super\ Mario\ Bros.\ \(Japan\,\ USA\).nes ",shell=True,stdout = null,stderr=null)
    time.sleep(8)
    process = subprocess.Popen("../nes localhost "+str(port),shell=True,stdout=fdout, stdin = subprocess.PIPE)
    time.sleep(0.1)
    if process.returncode != None:
        time.sleep(1)
        process = subprocess.Popen("../nes localhost " + str(port), shell=True, stdout=fdout, stdin=subprocess.PIPE)
    cmd = 1

    steps = 0;
    exe = 0
    #iterTime = 0
    loopTime = 0
    maxx = 0
    maxflag = 0
    coins = 0

    #oldresult = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    while True:
        #time.sleep(0.1)
        cin = fdin.readline()
        if exe == 0 and cin == "command: ":
            loopTime = 0
            process.stdin.write(bytes(trans[cmd] + "\n",'utf-8'))
            process.stdin.flush()
            exe = 1
            continue
        #第一种情况

        if cin == '' or cin == None or cin[0] != '{':
            loopTime = loopTime + 1
            if loopTime == 1000000:
                process.terminate()
                fce.terminate()
                os.system('rm result' + str(port) + '.txt ' + str(port) + ".lua")
                return evaluate(genome)
            continue

        #无效信息
        else:

            #print("output:" + cin)
            while True:
                try:
                    st = json.loads(cin)
                except:
                    time.sleep(0.5)
                    tmp = fdin.readline()
                    cin = cin + tmp
                    continue
                break

            if maxx < st['mario']['x']:
                maxflag = 0
                maxx = st['mario']['x']
            else:
                maxflag = maxflag + 1

            coins = max(st['coins'],coins)
            steps = steps+1

            if st['lives'] < 2 or maxflag > 60:   #这里maxflag>A还需要调整参数
                process.terminate()
                fce.terminate()
                os.system('rm result'+str(port)+'.txt '+ str(port) +".lua")
                #return 3*maxx+200*coins-steps
                return  maxx
            else:
                tmp = st['tiles']
                result = []
                oldresult = []
                for key in keyvalve:
                    for i in tmp[key]:
                        #oldresult.append(i)
                        result.append(i)
                result.append(cmd)
                #result.append(st['mario']['x'])
                #result.append(st['coins'])
                #oldresult.append(cmd)
                #oldresult.append(st['mario']['x'])
                #oldresult.append(st['coins'])
                #if iterTime == 50:            #一开始实验的时候限制总步数
                 #   process.terminate()
                  #  fce.terminate()
                   # os.system('rm result' + str(port) + '.txt ' + str(port) +".lua")
                    #return maxx
                    # result.append(1.0)
                #print("Output:" + str(st['mario']))
                net.Input(result)
                net.Activate()
                o = net.Output()
                if o[0]>o[1] and o[0]>o[2] and o[0]>o[3]:
                    cmd = 0
                elif o[1]>o[2] and o[1]>o[3] and o[1]>o[0]:
                    cmd = 1
                elif o[2]>o[3] and o[2]>o[1] and o[2]>o[0]:
                    cmd = 2
                else:
                    cmd = 3

            exe = 0


pool = Pool(8)
log = 0
aaa = [pop.GetBestGenome(),pop2.GetBestGenome(),pop3.GetBestGenome(),pop4.GetBestGenome(),pop5.GetBestGenome()]
pool.map(evaluate, aaa)
#evaluate(pop.GetBestGenome())
pool.close()
pool.join()
