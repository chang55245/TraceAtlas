
import matplotlib.pyplot as plt
import json
import os
import pandas as pd

# file = open("gantt_test_radar.json")
file = open("CEDR-DAG/testCase.json")
inputjs = json.load(file)


kernelInsdic = {}
for kernel in inputjs["KernelInstanceMap"]:
    kernelInsdic[kernel[0]] = kernel[1]

df = pd.DataFrame(columns=("KIid", "Kid", "type", "start", "end", "access", "accessed data", "reuse"))


patterns = [ "|" , "\\" , "/" , "+" , "-", ".", "*","x", "o", "O" ]

pattern = {"load": patterns[0], "store": patterns[1], "loadstore": patterns[0]}
color = {"load": "red", "store": "green", "loadstore": "yellow"}
# read the json into a data frame
for loadStore, idTuple in inputjs["tuplePerInstance"].items():
    for id, startAddrTuple in idTuple.items():
        for startAddr, tuple in startAddrTuple.items():
            dic = {"KIid": int(id), "Kid": kernelInsdic[int(id)] if kernelInsdic[int(id)] != "" else "none",
                   "type": loadStore, "start": tuple["1"], "end": tuple["2"], "accessed data": tuple['3'],
                   "access": tuple['4'], "reuse": tuple['5']}
            df = df.append(dic, ignore_index=True)

# def ovelapWithFormer(start, end, list):
#     for i in list:
#         if max(i[0], start) <= min(i[1], end):
#             outstart = max(i[0], start)
#             outend = min(i[1], end)
#             i[0] = min(i[0], start)
#             i[1] = max(i[1], end)
#             return [1, outstart, outend]
#
#     list.append([start, end])
#     return [0, 0, 0]
#
#
# for i, task in enumerate(df.groupby("KIid")):
#     addrList = []
#     for index, r in task[1].iterrows():
#         if addrList.__len__() != 0:
#             # if min(r[1]["start"].values[0],start) == start or max(r[1]["end"].values[0],end) == end:
#             res = ovelapWithFormer(r["start"], r["end"], addrList)
#             if res[0]:
#                 dic = {"KIid": r["KIid"], "Kid": r["Kid"],
#                        "type": "loadstore", "start": res[1], "end": res[2]}
#                 df = df.append(dic, ignore_index=True)
#         else:
#             addrList.append([r["start"], r["end"]])

df = df[df.start < 32820316768]
df.drop('accessed data', axis=1, inplace=True)
df["Diff"] = df.end - df.start
# index_names = df[df['Diff'] < 64].index
# df.drop(index_names, inplace=True)
# index_names = df[df['Kid'] == "-1"].index
# df.drop(index_names, inplace=True)

df.sort_values("KIid", inplace=True)
df = df.reset_index(drop=True)
print(df.to_string())
# adding distance between tuples


fig, ax = plt.subplots(figsize=(6, 3))
labels = []

# for avoiding duplicated legend
duplenged = []
for i, task in enumerate(df.groupby("KIid")):
    lab = task[0]
    labels.append(lab)
    for r in task[1].groupby("type"):
        data = r[1][["start", "Diff", "access", "Kid"]]
        access = r[1]["access"]
        a = data.values[:, 0:2]
        if r[0] not in duplenged:
            duplenged.append(r[0])
            legendin = r[0]
        else:
            legendin = ""
        if r[0] == "load":
            # ax.broken_barh(data.values[:, 0:2], (i, 0.4), color="white",edgecolor='black', label=legendin, hatch=patterns[5])
            ax.broken_barh(data.values[:, 0:2], (i, 0.4), color=color[r[0]], label=legendin,hatch=pattern[r[0]])

            for k in data.values:
                ax.text(  # position text relative to data
                    k[0], i + 0.2, (k[1], k[2], k[3]),  # x, y, text,
                    ha='center', va='bottom',  # text alignment,
                    transform=ax.transData,size = 10  # coordinate system transformation
                )
        elif r[0] == "store":
            # ax.broken_barh(data.values[:, 0:2], (i - 0.4, 0.4), color="white",edgecolor='black', label=legendin, hatch=patterns[1])
            ax.broken_barh(data.values[:, 0:2], (i - 0.4, 0.4), color=color[r[0]], label=legendin)
            for k in data.values:
                plt.text(  # position text relative to data
                    k[0], i - 0.2, (k[1], k[2], k[3]),  # x, y, text,
                    ha='center', va='bottom',  # text alignment,
                    transform=ax.transData,size = 10  # coordinate system transformation
                )



#--------------------------------thickness encoding--------------------------------
# fig, ax = plt.subplots(figsize=(6, 3))
# labels = []
# labelRange = []
# # for avoiding duplicated legend
# duplenged = []
# height_counter = 0
# for j, BBtask in enumerate(df.groupby("KIid")):
#     lab = str(BBtask[0])
#     for r in BBtask[1].groupby("type"):
#         labAddIndex = lab + "--" + str(r[0])
#         labels.append(labAddIndex)
#         data = r[1][["start", "Diff", "access", "Kid"]]
#         access = r[1]["access"]
#         # a = data.values[:, 0:2]
#         # a = access.values[0]
#         loadTuple = []
#         loadAccess = 0
#         storeTuple = []
#         storeAccess = 0
#         if r[0] not in duplenged:
#             duplenged.append(r[0])
#             legendin = r[0]
#         else:
#             legendin = ""
#
#         if r[0] == "load":
#             loadTuple = data.values[:, 0:2]
#             loadAccess = access.values[0]
#             ax.broken_barh(loadTuple, (height_counter, loadAccess), color=color["load"],
#                            label="" if legendin != "load" else "load")
#             labelRange.append(height_counter + loadAccess * 0.5)
#
#
#             for k in data.values:
#                 plt.text(  # position text relative to data
#                     k[0], height_counter + loadAccess * 0.5, (k[1], k[2],k[3]),  # x, y, text,
#                     ha='center', va='bottom',  # text alignment,
#                     transform=ax.transData  # coordinate system transformation
#                 )
#             height_counter = height_counter + loadAccess
#
#         elif r[0] == "store":
#             storeTuple = data.values[:, 0:2]
#             storeAccess = access.values[0]
#             ax.broken_barh(storeTuple, (height_counter, storeAccess), color=color["store"],
#                            label="" if legendin != "store" else "store")
#             labelRange.append(height_counter + storeAccess*0.5)
#
#             for k in data.values:
#                 plt.text(  # position text relative to data
#                     k[0], height_counter + storeAccess * 0.5, (k[1], k[2],k[3]),  # x, y, text,
#                     ha='center', va='bottom',  # text alignment,
#                     transform=ax.transData  # coordinate system transformation
#                 )
#             height_counter = height_counter + storeAccess





df.sort_values("start", inplace=True)
print(df.to_string())

# for thickness change
# ax.set_yticks(labelRange)
ax.set_yticks(range(len(labels)))
ax.set_yticklabels(labels)
ax.tick_params(axis='both', which='major', labelsize=20)
plt.xlabel("Address Accessed",size = 25)
plt.ylabel("Node Index",size = 25)
# plt.ylabel("Kernel Instance Index",size = 25)
plt.legend(bbox_to_anchor=(1.01, 0.89), loc='center left', prop={'size': 20});
# ax.set_xlim(5555472, 5636784)
# ax.set_ylim(29, 69)
# plt.tight_layout()



plt.show()

# inp = u"""a0:86:c6:52:4e:e8,0.006568,0.006620,Out
# a0:86:c6:52:4e:e8,0.006663,0.006695,In
# a0:86:c6:52:4e:e8,0.008089,0.008141,Out
# a0:86:c6:52:4e:e8,0.008185,0.008217,In
# 01:00:5e:00:00:fb,0.033096,0.035016,Out
# 33:33:00:00:00:fb,0.034997,0.037077,Out
# 01:00:5e:7f:ff:fa,0.039969,0.042057,Out
# ff:ff:ff:ff:ff:ff,0.059823,0.061639,Out
# a0:86:c6:52:4e:e8,0.068865,0.068917,Out
# a0:86:c6:52:4e:e8,0.068962,0.068994,In
# a0:86:c6:52:4e:e8,0.083492,0.083544,Out
# a0:86:c6:52:4e:e8,0.083588,0.083620,In"""
#
# import pandas as pd
# import io
# import matplotlib.pyplot as plt
#
# df = pd.read_csv(io.StringIO(inp), header=None, names=["Task", "Start", "Finish", "Resource"] )
# df["Diff"] = df.Finish - df.Start
#
# color = {"In":"turquoise", "Out":"crimson"}
# fig,ax=plt.subplots(figsize=(6,3))
#
# labels=[]
# for i, task in enumerate(df.groupby("Task")):
#     labels.append(task[0])
#     for r in task[1].groupby("Resource"):
#         data = r[1][["Start", "Diff"]]
#         ax.broken_barh(data.values, (i-0.4,0.8), color=color[r[0]] )
#
# ax.set_yticks(range(len(labels)))
# ax.set_yticklabels(labels)
# ax.set_xlabel("time [ms]")
# plt.tight_layout()
# plt.show()
