import numpy as np

target_item_id = 0 #zero-base
item_num = 1682
stranger_num = 10
friend_num = 70
alpha = 8
beta = 2

np.random.seed(2016)

#user u:
index_vec = np.zeros(item_num, np.int32)
index_vec[target_item_id]=1

'''
#strangers:
# pesudo rating matrix
#strangers = np.random.randint(low=0, high=5, size=(stranger_num, item_num))
#e.g. item_num = 10, stranger_num = 5
strangers = [[2, 3, 3, 0, 0, 2, 4, 1, 1, 1],
             [0, 2, 1, 0, 4, 3, 3, 2, 4, 2],
             [1, 2, 1, 1, 0, 3, 4, 1, 0, 0],
             [2, 2, 1, 1, 4, 0, 3, 0, 1, 2],
             [3, 1, 2, 0, 4, 2, 1, 2, 4, 4]]
'''

strangers = np.random.randint(5,size=(stranger_num,item_num))
rmv = np.random.randint(5,size=(stranger_num,item_num)) #make a matrix with density 20% zero
strangers[rmv!=0]=0; #make the rating matrix density 20%
#print strangers
strangers = np.multiply(strangers,16)

np.savetxt('strangers.txt', strangers,  fmt='%d')

msk = strangers > np.zeros((stranger_num,item_num),np.int32)
msk = msk.astype(np.int32)
mean_t = np.mean(strangers,1).astype(np.int32)
#print mean_t

#qt,b for all strangers
qt = np.zeros(stranger_num,np.int32)
idx = 0
for one in msk:
    qt[idx] = np.dot(one,index_vec)
    idx += 1

dt = np.sum(qt)
print "dt: ", dt
#rt = qt,b(Rt.Ib-mean_rt) and stage3 for users
idx = 0
rt = np.zeros(stranger_num,np.float)
for one in strangers:  
    rt[idx] = qt[idx]*(np.dot(one,index_vec)-mean_t[idx])
    #print idx, "qt: ", qt[idx], "all:", np.dot(one,index_vec)-mean_t[idx], mean_t[idx], "<<", rt[idx], qt[idx]*(np.dot(one,index_vec)-mean_t[idx])
    idx += 1

nt = np.sum(rt)
print "nt: ", nt

'''
#friends:
# pesudo rating matrix
#friends = np.random.randint(low=0, high=5, size=(friend_num, item_num))
#e.g. item_num = 10, friend_num = 15
'''

friends = np.random.random_integers(5,size=(friend_num,item_num))
rmv = np.random.randint(5,size=(friend_num,item_num))
friends[rmv!=0]=0;
friends = np.multiply(friends,16)

np.savetxt('friends.txt', friends,  fmt='%d')

mean_f = np.mean(friends,1).astype(np.int32)
#print mean_f

mskf = friends > np.zeros((friend_num,item_num),np.int32)
mskf = mskf.astype(np.int32)

#similarity, w, use corr to get real pearson coe
w = np.ones((friend_num,friend_num),np.int32)*8

np.savetxt('sim.txt', w,  fmt='%d')

#rf = qf,b(Rf.Ib-mean_rf) and stage3 for users
qf = np.zeros(friend_num,np.int32)
idx = 0
for one in mskf:
    qf[idx] = np.dot(one,index_vec)
    idx += 1

df = np.sum(qf)
print "df: ", df

#rt = qt,b(Rt.Ib-mean_rt) and stage3 for users

idx = 0
rf = np.zeros(friend_num,np.int32) #np.int32 will limit all ints to 8-bits, so all above 256 will be 0
for one in friends:
    #rf[idx] = qf[idx]*(np.dot(one,index_vec)-mean_f[idx])*w[1,idx]
    rf[idx] = qf[idx]*(np.dot(one,index_vec)-mean_f[idx])*w[1,idx]
    #print "idx:",idx, "npdot:", np.dot(one,index_vec), "mean:", mean_f[idx], "qf", qf[idx], "w:", w[1,idx], "rf:", rf[idx]
    idx += 1

nf = np.sum(rf)

print "nf: ", nf

#pure stage 3:
xu = beta*nt*df+alpha*nf*dt
yu = (alpha+beta)*dt*df

print "xu: ", xu, "; yu: ", yu

pub = float(xu)/float(yu)

print "prediciton - tau ", pub

