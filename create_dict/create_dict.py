import csv, random, colorsys, math
from igraph import Graph, Plot
from tqdm import tqdm

LENGTH = 5
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
    g = Graph()
    orig_vertex = g.add_vertex(name="__origin")
    end_vertex = g.add_vertex(name="__end")
    for l, layer in tqdm(enumerate(tree), total=LENGTH, unit="layers", ncols=128, colour="green", leave=False):
        for edge in tqdm(layer, total=len(layer), unit="edges", ncols=128, colour="yellow", leave=False):
            if l < LENGTH-1:
                child_str = get_child_string(edge, tree[l+1])
                if not child_str in g.vs["name"]:
                    g.add_vertex(name=child_str)

            for p in edge["parents"]:
                parent = orig_vertex if l == 0 else get_child_string(p, layer)
                if l == LENGTH - 1:
                    e = g.add_edge(parent, end_vertex)
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
        "vertex_color": "#fff",
        "vertex_size": 10
    }

    layout = g.layout("rt", root=[orig_vertex.index])
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

def write_flat_to_c(filename, flat, dict_index):
    out = f"const uint8_t dict_{dict_index}" + "[] = {"
    for i, edge in tqdm(enumerate(flat), total=len(flat), unit="edges", ncols=128, colour="yellow", leave=False):
        has_more = not (i == len(flat)-1 or flat[i+1]["parents"] != edge["parents"])
        letter_dec = ord(edge["letter"])-97
        letter_bin = format(letter_dec, "05b")
        index_dec = find_child_index(flat, edge)
        index_bin = format(index_dec, "018b")

        entry = f"{ 1 if has_more else 0 }{letter_bin}{index_bin}"

        e1 = f"0b{entry[0:8]}"
        e2 = f"0b{entry[8:16]}"
        e3 = f"0b{entry[16:24]}"
        
        out += f"{hex(int(e1, 2))},{hex(int(e2, 2))},{hex(int(e3, 2))}, "
    
    out += "};"

    with open(filename, "w") as f:
        f.write(out)
        f.close()

def write_to_c(filename, tree, splitnum=1):
    flat = flatten_tree(tree)
    length = len(flat)
    step = round(length/splitnum + 0.5)

    # split into chunks
    chunks = []
    for i in range(0, length, step):
        chunks.append(flat[i:i + step])

    for i, chunk in tqdm(enumerate(chunks), total=splitnum, unit="chunks", ncols=128, colour="green", leave=False):
        write_flat_to_c(f"{filename}_{i}.c", chunk, i)

if __name__ == "__main__":
    # get a list of all LENGTH letter words
    words = []
    with open(FILENAME) as f:
        reader = csv.reader(f)
        for row in reader:
            length = len(row[0])
            if length == LENGTH:
                words.append(row[0])

    #########################################################
    # create the big word data dict table                   #
    # searching starts at index 0 (empty node)              #
    # each edge is 20 bits long and structured as follows:  #
    # CLLLLLII IIIIIIII IIIIIIII                            #
    # C -> node has more edges?                             #
    # L -> Letter                                           #
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
    for l in range(LENGTH):
        tree.append([])

    # create edges
    for w in words:
        parent = False
        for i, l in enumerate(w):
            new_edge = {
                "parents": [parent],
                "letter": w[i:i+1],
                "id": str(random.randint(0, 9999999999999))
            }
            parent = new_edge;
            tree[i].append(new_edge)

    print("✅ extensive tree created")
    print("⏳ starting same parent elimination")

    # 2. same parent elimination
    for l, layer in tqdm(enumerate(tree), total=LENGTH, unit="layers", ncols=128, colour="green", leave=False):
        # sort edges by letter and parent
        edges_sorted = {}
        for edge in layer:
            # create parent string (concat of all parent ids)
            parent_str = get_parent_string(edge, l)

            key = str(edge["letter"] + parent_str)
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
                if l < LENGTH - 1:
                    move_children(edge, main, tree[l + 1])
                layer.remove(edge)

    print("✅ same parent elimination done")
    print("⏳ starting same children elimination")

    # 3. same children elimination
    for l in tqdm(reversed(range(LENGTH)), total=LENGTH, unit="layers", ncols=128, colour="green", leave=False):
        layer = tree[l]
        # sort edges by letter and children
        # exclude edges with "siblings"    
        edges_sorted = {}
        for edge in tqdm(layer, unit="edges", total=len(layer), ncols=128, colour="yellow", leave=False):
            siblings = get_siblings(edge, layer)
            if len(siblings) > 0: continue
            # create child string (concat of all child ids)
            child_str = ""
            if l == LENGTH - 1:
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

    write_to_c(OUTFILE, tree, 5)

    print("✅ output file written")
    print("⏳ starting graph drawing")

    draw_graph(tree)