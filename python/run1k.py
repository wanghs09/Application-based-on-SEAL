# author: jun.wang (at) uni.lu
# We will not try to improve this implementation. But if you find any bug, please feel free to contact me.

import sys
import datetime
from os import path, makedirs
from random import shuffle
import numpy as np
import pickle

sys.path.append("./src")
from models import Model, FriendsModel
from similarities import CosineSimilarity, JaccardSimilarity
from dataset import get_friends_data, get_user_item_matrix, get_user_item_matrix_sub, \
                    get_reputation, get_original_user_item_matrix,get_original_UI, \
                    get_moive100k, construct_data_set
from evaluators import GlobalJaccardKfold,FriendsJaccardKfold,FriendsCosineKfold, \
                       GlobalCosineKfold,NMFKfold, FriendsReputationKfold, \
                       GlobalReputationKfold, FriendStrangerKfold,\
                       JphKfold, EsoricsSingleUserValidation
from evaluators import root_mean_square_error, mean_absolute_error

from predictors import Friends_Strangers


def get_friends_from_ml100k():
    dst_file = "./src/dataset/movie100k/ml100kfriend.dat"
    if path.isfile(dst_file):
        friend_data = pickle.load(open(dst_file, "rb"))
        return friend_data

    raw_data = get_moive100k(True);
    raw_model = Model(raw_data);
    cosine_sim = CosineSimilarity(raw_model)
    friend_data = {}

    for user_id in raw_model.get_user_ids():
        neighbors = cosine_sim.get_similarities(user_id)[:250]
        user_ids, x = zip(*neighbors)
        user_ids = list(user_ids)
        shuffle(user_ids)
        # note: 
        # Randomly choose 150 out of 250 neighbors as friends.
        # In such case, systems is able to (possiblly) choose strangers which 
        # are in top-250 similar users, but with a probability slightly
        # smaller than friends selection.
        friend_data[user_id] = user_ids[:150] 

    pickle.dump(friend_data,open(dst_file, "w"),protocol=2)
    return friend_data

def calculate_cosine_similarity_friends(raw_data,friend_data):

    friend_model = FriendsModel(raw_data,friend_data)
    cosine_sim = CosineSimilarity(friend_model)
    c_sims = {}

    for ky in friend_model.get_friends_roster():
        f_ids = friend_model.get_friends(ky)
        sorted_sims = cosine_sim.get_similarities(ky, f_ids)
        c_sims[ky] = sorted_sims
    return c_sims


def calculate_cosine_similarity_global(raw_data,friend_data):
    friend_model = FriendsModel(raw_data,friend_data)
    cosine_sim = CosineSimilarity(friend_model)
    c_sims = {}

    for u_id in friend_model.get_friends_roster():
        f_num = len(friend_model.get_friends(u_id))
        target_ids = friend_model.get_strangers_in_roster(u_id)
        sorted_sims = cosine_sim.get_similarities(u_id,target_ids)
        c_sims[u_id] = sorted_sims[:f_num]
    return c_sims


def get_metrics_methods():
    return {"RMSE":root_mean_square_error,
            "MAE": mean_absolute_error}

def calculate_cosine_rmse_friends(raw_data,friend_data):
    mtrs = get_metrics_methods()
    fckx = FriendsCosineKfold(5,raw_data,friend_data,mtrs)
    fckx.cross_validate()
    return


def calculate_cosine_friends_strangers(raw_data,friend_data, f_t_shape):
    f_ratios = [0.5,0.6,0.7,0.8,0.9,1.0]
    mtrs = get_metrics_methods()
    results = {}

    for f_n, t_n in f_t_shape:
        for ratio in f_ratios:
            ky = str(f_n)+"-"+str(t_n)+"-"+str(ratio)
            print "start cross validation > friendN-strangerN-ratio: ", ky
            ftkflod = FriendStrangerKfold(5,raw_data,friend_data,mtrs,f_n,t_n,ratio)
            mean_rs, bound, mae_std, max_mae, min_mae = ftkflod.cross_validate()
            one_rs =  [round(mean_rs,4), round(bound,4), round(mae_std,4),round(max_mae,4), round(min_mae,4)]
            results[ky] = one_rs
            print ">> mean: ", round(mean_rs,4), "; max diverse: ", round(bound,4), "; STD: ", round(mae_std,4), "; max MAE, ", round(max_mae,4), "; min MAE: ", round(min_mae,4)
    return results

def single_influence(raw_data, friend_data,testfriend,filename):
    if testfriend:
        f_ns = [11, 31] # randomly remove one when selecting friends
        t_n = 10
    else:
        f_ns = [10, 30]
        t_n = 11 # randomly remove one when selecting strangers

    ratio = 0.8
    for f_n in f_ns:
        print "50X(5-fold CV) for friends num: ", f_n
        esvlidation = EsoricsSingleUserValidation(5,raw_data,friend_data,f_n,t_n,ratio,testfriend)
        results = esvlidation.cross_validate()
        tname = filename+str(f_n)
        np.savetxt(tname, results, delimiter=',')
        #print results

def jhk_friends(raw_data, friend_data, f_s):
    mtrs = get_metrics_methods()
    for  f_num in f_s:
       print "start cross validation >  friend num: ", f_num
       jkx = JphKfold(5,raw_data,metrics=mtrs,friends_data=friend_data, friends_num=f_num)
       mean_rs, bound, mae_std, max_mae, min_mae = jkx.cross_validate()
       
       print ">> mean: ", round(mean_rs,4), "; max diverse: ", round(bound,4), "; STD: ", round(mae_std,4), "; max MAE, ", round(max_mae,4), "; min MAE: ", round(min_mae,4)

def pure_single_friend_influence(raw_data, friend_data, filename):
    f_ns = [3,6,11] #will randomly exclude one
    t_n= 0
    ratio = 1
    for f_n in f_ns:
        print "50X(5-fold CV) for friends num: ", f_n
        esvlidation = EsoricsSingleUserValidation(5,raw_data,friend_data,f_n,t_n,ratio,True)
        results = esvlidation.cross_validate()
        tname = filename+str(f_n)
        np.savetxt(tname, results, delimiter=',')

def save_list_to_file(filename, ldat):
    np.savetxt(filename,ldat,fmt="%d")

    #with open(filename, 'w') as f:  
    #    f.writelines("%s\n" % l for l in ldat)

def generate_simulating_data(raw_data,friend_data, user_id, f_num, t_num,filelocation):

    if not user_id in friend_data:
        print "invalid user_id!"
        return

    if not path.exists(filelocation):
        makedirs(filelocation)

    friend_model = FriendsModel(raw_data,friend_data)
    cosine_sim = CosineSimilarity(friend_model)
    fs = Friends_Strangers(cosine_sim, f_num, t_num)
    friends = fs.get_rand_friends(user_id)
    strangers = fs.get_rand_strangers(user_id)
    f_ids, f_sims = zip(*friends)
    t_ids, _ = zip(*strangers)

    f_sims = np.dot(f_sims,8).astype(np.int32).tolist() #for HE
    #print f_sims
    f1 = filelocation+"similarity_"+str(user_id)+"_"+str(f_num)+".dat"
    save_list_to_file(f1, [f_sims]) #friends similarity 

    diskfriend = []
    for f_id in f_ids:
        vec = friend_model.get_dense_user_vector(f_id)
        vec = np.dot(vec,16).astype(np.int32).tolist()#for HE
        #vec.tolist()
        diskfriend.append(vec)
    f2 = filelocation+"friend_"+str(user_id)+"_"+str(f_num)+".dat"
    save_list_to_file(f2, diskfriend) #friends rating data


    diskstranger = []
    t_ids, __ = zip(*strangers)

    for t_id in t_ids:
        vec = friend_model.get_dense_user_vector(f_id)
        vec = np.dot(vec,16).astype(np.int32).tolist() #for HE
        diskstranger.append(vec)
    f3 = filelocation+"stranger_"+str(user_id)+"_"+str(t_num)+".dat"
    save_list_to_file(f3, diskstranger) #friends rating data



def Pure_Friend_Influence_on_10_FMT():
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    filename = "pure_friend_dif_10fmt_"
    pure_single_friend_influence(raw_data,friend_data,filename)

def Pure_Friend_Influence_on_MovieLens():
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    filename = "pure_friend_dif_ml_"
    pure_single_friend_influence(raw_data,friend_data,filename)

def MAE_on_10_FMT():
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    f_ts = [(10,10),(20,10),(30,10),(40,10),(50,10)]
    calculate_cosine_friends_strangers(raw_data, friend_data, f_ts) 

def MAE_on_MovieLens():
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    f_ts = [(10,10),(20,10),(30,10),(40,10),(50,10),(60,10),(70,10),(80,10),(90,10),(100,10)]
    calculate_cosine_friends_strangers(raw_data, friend_data, f_ts)

def Single_Friend_Influence_on_10_FMT():
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    filename = "friend_dif_10fmt_"
    single_influence(raw_data,friend_data,True,filename)

def Single_Friend_Influence_on_MovieLens():
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    filename = "friend_dif_ml_"
    single_influence(raw_data,friend_data,True,filename)

def Single_Stranger_Influence_on_10_FMT():
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    filename = "stranger_dif_10fmt_"
    single_influence(raw_data,friend_data,False,filename)

def Single_Stranger_Influence_on_MovieLens():
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    filename = "friend_dif_ml_"
    single_influence(raw_data,friend_data,False,filename)

def JPH_on_10_FMT():
    f_s = [10, 20 ,30, 40, 50]
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    jhk_friends(raw_data, friend_data, f_s)

def JPH_on_MovieLens():
    f_s = [20, 40 ,60, 80, 100]
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    jhk_friends(raw_data, friend_data, f_s)

def Generate_Simulating_Data_on_MovieLens(user_id,f_num, t_num):
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    friend_model = FriendsModel(raw_data,friend_data)
    cosine_sim = CosineSimilarity(friend_model)
    fs = Friends_Strangers(cosine_sim, f_num, t_num)
    friends = fs.get_rand_friends(user_id)
    strangers = fs.get_rand_strangers(user_id)

def Generate_Simulating_Data_on_10_FMT(user_id):
    filelocation = "./FMT10/"
    friend_data = get_friends_data()
    raw_data = get_user_item_matrix()
    generate_simulating_data(raw_data,friend_data,user_id,30,10,filelocation)


def Generate_Simulating_Data_on_MovieLens(user_id):
    user_id = int(user_id)

    filelocation = "./ML100K/"
    friend_data = get_friends_from_ml100k()
    raw_data = get_moive100k()
    generate_simulating_data(raw_data,friend_data,user_id,70,10,filelocation)

def Show_Users_on_10_FMT():
    indx = 0
    friend_data = get_friends_data()
    for ky in friend_data:
        print ky
        indx += 1
        if indx > 20:
            break
    print "..."

def Show_Users_on_MovieLens():
    indx = 0
    friend_data = get_friends_from_ml100k()
    for ky in friend_data:
        print ky
        indx += 1
        if indx > 20:
            break
    print "..."


def Show_Data_Info():
    print "Location: >>"
    print "    FMT original files: ./src/dataset/expdata. "
    print "    Filted K-FMT files (binary): ./src/dataset/mid_data. "
    print "Dataset description: >> "
    print "    1. This data set is built based on MovieTweetings dataset: https://github.com/sidooms/MovieTweetings."
    print "    2. We crawled the friends information for each user in MovieTweetings dataset since it does not have."
    print "    There are three files in the dataset: ratings.dat, users.dat, friends.dat."
    print "    <ratings.dat> records users' rating information on moives."
    print "    format>> user_id::movie_id::rating::rating_timesstamp"
    print "    The ratings contained in the tweets are scaled from 0 to 10."
    print "    <users.dat> contains the mapping for user_id to real twitter_id."
    print "    format>> {user_id1:twitter_id1, user_id2:twitter_id2, ...}"
    print "    <friends.dat> contains each user's friends inforamtion (twitter_id)."
    print "    format>> {user_id1:[twitter_id9,twitter_id17,...,twitter_idn],user_id2:[twitter_id10,...],user_id3:[twitter_id100,...],...}"


def note():
    print "Please indicate target: "
    print "0 -- filter FMT data set by friend num (default value: 10) and rating number (default value: 10)"
    print "1 -- MAE test on 10-FMT "
    print "2 -- MAE test on MovieLens "
    print "3 -- JPH test on 10-FMT "
    print "4 -- JPH test on MovieLens "
    print "5 -- Single friend influence test on 10-FMT "
    print "6 -- Single friend influence test on MovieLens "
    print "7 -- Single stranger influence test on 10-FMT "
    print "8 -- Single stranger influence test on MovieLens "
    print "9 -- Single friend influence without involving any stranger on 10-FMT"
    print "10 -- Single friend influence without involving any stranger on MovieLens"
    print "11 -- Create protocol similation files from 10-FMT. You should input user_id (FMT is twitter ID)"
    print "12 -- Create protocol similation files from MovieLens. You should input user_id (FMT is twitter ID)"
    print "13 -- show a number of user ids of FMT"
    print "14 -- show a number of user ids of MovieLens"
    print "99 -- Show FMT dataset information"

def nllfunc():
    return


for i in range(1, 1001):
    Generate_Simulating_Data_on_MovieLens(i)