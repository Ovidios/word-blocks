import csv, random, colorsys, math, sys
from igraph import Graph, Plot
from tqdm import tqdm

MIN_LENGTH = 3
MAX_LENGTH = 6
FILENAME = "scrabble.csv"
OUTFILE = "dict"

def hsv(h, s, v):
    r, g, b = colorsys.hsv_to_rgb(h, s, v)
    return (r*255, g*255, b*255)

def get_parent_string(edge, l_index):
    parent_str = ""
    if l_index == 0:
        return "__origin"
    else:
        for parent in edge["parents"]:
            parent_str += "__" + parent["id"]
    return parent_str

def get_child_string(edge, child_layer):
    children = get_children(edge, child_layer)
    child_str = ""
    for c in children: child_str += "__" + c["id"]
    return child_str

def draw_graph(tree):
    print(len(tree))
    g = Graph()
    orig_vertex = g.add_vertex(name="__origin")
    orig_vertex["layer"] = 0
    end_vertex = g.add_vertex(name="__end")
    end_vertex["layer"] = len(tree) + 1
    for l, layer in tqdm(enumerate(tree), total=len(tree), unit="layers", ncols=128, colour="green", leave=False):
        for edge in tqdm(layer, total=len(layer), unit="edges", ncols=128, colour="yellow", leave=False):
            if not edge["end"]:
                child_str = get_child_string(edge, tree[l+1])
                if not child_str in g.vs["name"]:
                    v = g.add_vertex(name=child_str)
                    v["layer"] = l+1

            for p in edge["parents"]:
                parent = orig_vertex if l == 0 else get_child_string(p, layer)
                if edge["end"]:
                    e = g.add_edge(parent, end_vertex)
                    e["end"] = True
                else:
                    e = g.add_edge(parent, child_str)
            e["letter"] = edge["letter"]
            e["parents"] = get_parent_string(edge, l)
            e["curved"] = False

    num_edges = g.ecount()

    style = {
        "edge_label":  g.es["letter"],
        "edge_color": ["rgb" + str(hsv(i/num_edges * 2 * math.pi, 0.75, 1)) for i, _ in enumerate(g.es)],
        "edge_label_color": ["rgb" + str(hsv(i/num_edges * 2 * math.pi, 1, 0.75)) for i, _ in enumerate(g.es)],
        "edge_label_size": 16,
        "vertex_color": ["#f00" if i == 0 else ("#00f" if i == (len(g.vs)) else "#fff") for i, _ in enumerate(g.vs)],
        "vertex_size": 10
    }

    layout = g.layout("sugiyama", layers="layer", maxiter=1000)
    plot = Plot(bbox=(0, 0, 1920, 1080), background="#201e2b")
    plot.add(g, layout=layout, bbox=(0,0,1920,1080), margin=(100, 100, 100, 100), **style)

    plot.show()

def move_children(parent, new_parent, child_layer):
    for edge in child_layer:
        if parent in edge["parents"]:
            edge["parents"].remove(parent)
            if not new_parent in edge["parents"]:
                edge["parents"].append(new_parent)
                edge["parents"].sort(key = lambda e : e["letter"] + e["id"])

def get_children(parent, child_layer):
    children = []
    for edge in child_layer:
        if parent in edge["parents"]:
            children.append(edge)
    return children

def get_siblings(edge, layer):
    siblings = []
    for parent in edge["parents"]:
        children = get_children(parent, layer)
        for child in children:
            if child != edge and not child in siblings:
                siblings.append(child)
    return siblings

def flatten_tree(tree):
    out = []
    for layer in tree:
        for edge in layer:
            out.append(edge)
    return out

def find_child_index(flat, edge):
    for i, e in enumerate(flat):
        if edge in e["parents"]:
            return i
    return 0

def flat_to_hex(flat):
    out = []
    for i, edge in tqdm(enumerate(flat), total=len(flat), unit="edges", ncols=128, colour="yellow", leave=False):
        has_more = not (i == len(flat)-1 or flat[i+1]["parents"] != edge["parents"])
        letter_dec = ord(edge["letter"])-97
        letter_bin = format(letter_dec, "05b")
        index_dec = find_child_index(flat, edge)
        index_bin = format(index_dec, "017b")

        entry = f"{ 1 if has_more else 0 }{letter_bin}{1 if edge['end'] else 0}{index_bin}"

        e1 = f"0b{entry[0:8]}"
        e2 = f"0b{entry[8:16]}"
        e3 = f"0b{entry[16:24]}"
        
        out.append(f"{hex(int(e1, 2))},{hex(int(e2, 2))},{hex(int(e3, 2))}, ")
    return out

def write_chunk_to_c(filename, chunk, dict_index):
    out = f"#include <gb/gb.h>\n\nconst uint8_t dict_{dict_index}" + "[] = {"
    #out = f"const int dict_{dict_index}" + "[] = {"
    
    for info in chunk:
        out += info

    out += "};"

    with open(filename, "w") as f:
        f.write(out)
        f.close()

def write_to_c(filename, tree, splitnum=1):
    flat = flatten_tree(tree)
    length = len(flat)
    step = round(length/splitnum + 0.5)
    print("step: ", step)

    hexinfo = flat_to_hex(flat)

    # split into chunks
    chunks = []
    for i in range(0, length, step):
        chunks.append(hexinfo[i:i + step])

    for i, chunk in tqdm(enumerate(chunks), total=splitnum, unit="chunks", ncols=128, colour="green", leave=False):
        write_chunk_to_c(f"{filename}_{i}.c", chunk, i)

if __name__ == "__main__":
    # get a list of all LENGTH letter words
    words = []
    with open(FILENAME) as f:
        reader = csv.reader(f)
        for row in reader:
            length = len(row[0])
            if length <= MAX_LENGTH and length >= MIN_LENGTH:
                words.append(row[0])

    #########################################################
    # create the big word data dict table                   #
    # searching starts at index 0 (empty node)              #
    # each edge is 20 bits long and structured as follows:  #
    # CLLLLLEI IIIIIIII IIIIIIII                            #
    # C -> node has more edges?                             #
    # L -> Letter                                           #
    # E -> End of Word?                                     #
    # I -> Index of next Node (0 if EOW)                    #
    #########################################################

    # 1. create an extensive tree
    # this contains all letters for each layer
    # we still have the concept of layers at this point
    # since all our words are the same length
    # and connections are only possible on the
    # same layer

    tree = []

    # create empty layers
    for l in range(MAX_LENGTH):
        tree.append([])

    # create edges
    for w in words:
        parent = False
        for i, l in enumerate(w):
            new_edge = {
                "parents": [parent],
                "letter": w[i:i+1],
                "id": str(random.randint(0, 9999999999999)),
                "end": i == len(w) - 1
            }
            parent = new_edge;
            tree[i].append(new_edge)

    print("✅ extensive tree created")
    print("⏳ starting same parent elimination")

    # 2. same parent elimination
    for l, layer in tqdm(enumerate(tree), total=len(tree), unit="layers", ncols=128, colour="green", leave=False):
        # sort edges by letter and parent and word end status
        edges_sorted = {}
        for edge in layer:
            # create parent string (concat of all parent ids)
            parent_str = get_parent_string(edge, l)

            key = str(edge["letter"] + parent_str + f"__{edge['end']}")
            if not key in edges_sorted.keys():
                edges_sorted[key] = []
            edges_sorted[key].append(edge)
        
        # unify the groups
        for key in tqdm(edges_sorted.keys(), unit="groups", ncols=128, colour="yellow", leave=False):
            if len(edges_sorted[key]) <= 1:
                continue
            main = edges_sorted[key][0]
            for i, edge in enumerate(edges_sorted[key]):
                if i == 0: continue
                if not edge["end"]:
                    move_children(edge, main, tree[l + 1])
                layer.remove(edge)

    print("✅ same parent elimination done")
    print("⏳ starting same children elimination")

    # 3. same children elimination
    for l in tqdm(reversed(range(len(tree))), total=len(tree), unit="layers", ncols=128, colour="green", leave=False):
        layer = tree[l]
        # sort edges by letter and children
        # exclude edges with "siblings"    
        edges_sorted = {}
        for edge in tqdm(layer, unit="edges", total=len(layer), ncols=128, colour="yellow", leave=False):
            siblings = get_siblings(edge, layer)
            if len(siblings) > 0: continue
            # create child string (concat of all child ids)
            child_str = ""
            if edge["end"]:
                child_str = "__end"
            else:
                children = get_children(edge, tree[l + 1])
                for child in children:
                    child_str += "__" + child["id"]

            key = str(edge["letter"] + child_str)
            if not key in edges_sorted.keys():
                edges_sorted[key] = []
            edges_sorted[key].append(edge)

        # unify the groups
        for key in tqdm(edges_sorted.keys(), unit="groups", total=len(edges_sorted.keys()), ncols=128, colour="yellow", leave=False):
            if len(edges_sorted[key]) <= 1:
                continue
            main = edges_sorted[key][0]
            for i, edge in enumerate(edges_sorted[key]):
                if i == 0: continue
                for p in edge["parents"]:
                    if not p in main["parents"]:
                        main["parents"].append(p)
                layer.remove(edge)

    print("✅ same children elimination done")
    print("⏳ writing output file")

    write_to_c(OUTFILE, tree, 12)

    print("✅ output file written")
    print("⏳ starting graph drawing")

    for i, l in enumerate(tree):
        print(i, len(l))

    draw_graph(tree)