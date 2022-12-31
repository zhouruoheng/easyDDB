from graphviz import Digraph

dot = Digraph(comment="query_tree", format="png");
color_dict = {
    1:'#EEA29A',
    2:'#FFEF96',
    3:'#D9ECD0',
    4:'#B7D7E8'
}

f = open("print.txt")
lines = f.readlines()

for line in lines:
    line = line.strip()
    line = line.replace('\t', '\n')
    res = line.split("|")
    dot.node(str(res[0]), res[1], style="filled",fillcolor=color_dict[int(res[2])])
    child = res[3].split(",")
    for c in child:
        if c=='': 
            continue
        dot.edge(str(c), str(res[0]))
dot.render(filename="print", view=False);
