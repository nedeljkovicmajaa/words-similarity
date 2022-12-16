#include <iostream>
#include <fstream>
#include <stddef.h>
#include <sstream>

using namespace std;

//struktura word - ime i pozicija na kojoj pocinju susedi te reci u nizu connections
struct word{
  int id;
  std::string name;
};

//struktura za cuvanje svih podataka potrebnih za dijkstru
struct str_dijkstra{
    string vertex;
    float short_dist;
    string prev;
    int isvis;
};

//struktura connection - linearizovani niz svih suseda svih cvorova
struct connection{
  std::string connection_name;
  float weight;
};

//struktura reprezentacija - dva niza za reprezentaciju i njihove duzine
struct representation{
    word* words;
    connection* connections;
    int num_words;
    int num_con;
};



//funkcija za alociranje memorije za niz bilo kog tipa
template <typename T>
T* make_array(int length){
    T* p = new T[length];
    if(p == nullptr){
        printf("Nije moguce alocirati niz");
    }
    return p;
}

//funkcija za brisanje niza bilo kog tipa iz memorije(ne slozenog)
template <typename T>
void delete_array(T* arr){
    delete[] arr;
}



int is_node_in(word* arr, int current_size, string node_name){
    for(int i = 0; i < current_size; i++){
        if(arr[i].name == node_name){
            return 1;
        }
    }
    return 0;
}

int is_connection_in(word* words, int word_size, connection* cons, int con_size, string name1, string name2){
    int pos1; int pos2;
    for(int i = 0; i < word_size-1; i++){
        if(words[i].name == name1){
            pos1 = words[i].id;
            pos2 = words[i+1].id;
        }
    }
    for(int i = pos1; i < pos2; i++){
        if(cons[i].connection_name == name2){
            return 1;
        }
    }
    return 0;
}

//funkcija za stampanje niza reci sa svim podacima strukturi unutar njega
void print_words(word* arr, int current_size){
    for(int i = 0; i < current_size; i++ ){
        cout << arr[i].id << ' ' << arr[i].name << '\n';
    }
    printf("\n");
}

//funkcija za stampanje niza konekcija sa svim podacima strukturi unitar njega
void print_connections(connection* arr, int current_size){
    for(int i = 0; i < current_size; i++ ){
        cout << arr[i].connection_name << ' ' << arr[i].weight << '\n';
    }
    printf("\n");
}



//funkcija za formiranje strukture svih parametara relevantnih za ucitavanje grafa
representation load_graph(string file){

    ifstream ifs(file);

    string line;
    //indeksi za izvlacenje reci iz stringa ogranicene razmacima
    int id = 0; int prev = 0; int prev1 = 0;

    //ucitavanje broja cvorova i konekcija u grafu
    getline(ifs, line); int num_node = stoi(line)+1;
    getline(ifs, line); int num_con = stoi(line);

    //alociranje memorije za nizove cvorova i konekcija (zadate su velicine)
    word* words = make_array<word>(num_node);
    connection* connections = make_array<connection>(num_con);

    //petlja za popunjavanje niza cvorova
    getline(ifs, line);
    for(int i = 0; i < num_node-1; i++){
        //line.find pronalazi indeks prvog razmaka nakon zadatog indeksa
        //njegovom indeksu se oduzima indeks prethodnog razmaka da bi se dobila duzina reci
        prev1 = line.find(' ', prev) - prev;

        //izdvaja se rec od indeksa datog razmaka, prev1 karaktera (koliko je i dugacka ta rec)
        string token = line.substr(prev, prev1);

        //indeks prvog karaktera nakon poslednje lociranog razmaka
        prev = prev + prev1 + 1;

        //dodavanje elementa u niz
        word elem;
        elem.id = id;
        elem.name = token;
        words[id] = elem;

        id++;
    }

    //dodavanje poslednjeg elementa u niz cvorova koji ne predstavlja rec, vec treba da pokazuje na kraj niza veza
    word elem;
    elem.id = id;
    elem.name = "";
    words[id] = elem;

    //pokazivaci za iteraciju kroz oba niza
    int con_iterator = 0; int word_iterator = 0;

    //iteracija kroz konekcije, popunjavanje niza veza i postavljanje id-jeva reci
    while(getline(ifs, line)){
        string first_word = line.substr(0, line.find(' '));
        int second_space = line.find(' ',line.find(' ')+1);
        string second_word = line.substr(line.find(' ')+1, second_space - line.find(' ') - 1);
        float num = stof(line.substr(second_space));

        connection con;
        con.connection_name = second_word;
        con.weight = num;
        connections[con_iterator] = con;

        //ukoliko neki element nema konekcije onda i on i sledeci imaju isti id
        while(words[word_iterator].name != first_word){
            word_iterator += 1;
            words[word_iterator].id = con_iterator;
        }
        con_iterator++;
    }

    //ukoliko neke reci na kraju nemaju konekcije one sve pokazuju na kraj
    word_iterator += 1;
    while(word_iterator < num_node){
        words[word_iterator].id = num_con;
        word_iterator += 1;
    }

    //formiranje konacne reprezentacije ucitanog grafa
    representation repr;
    repr.words = words;
    repr.connections = connections;
    repr.num_con = num_con;
    repr.num_words = num_node;

    return repr;
}

//funkcija za dodavanje novog cvora u graf
void add_node(word *&words, int &num_nodes, int num_con, string name){
    word new_w;
    new_w.id = num_con;
    new_w.name = " ";

    //alociranje memorije za novi niz i njegovo popunjavanje, gde se na kraj niza dodaje novi cvor (bez veza)
    word* new_words = make_array<word>(num_nodes + 1);
    for(int i = 0; i < num_nodes; i++){
        new_words[i] = words[i];
    }

    //poslednji element je prazan i pokazuje na kraj niza
    new_words[num_nodes-1].name = name;
    new_words[num_nodes] = new_w;

    //brisanje niza words i kopiranje novod niza tu
    delete_array(words);

    //menjanje duzine niza
    num_nodes += 1;

    words = make_array<word>(num_nodes);
    for(int i = 0; i < num_nodes; i++){
        words[i] = new_words[i];
    }

    //unistavanje novog niza
    delete_array(new_words);

}

//funkcija za brisanje cvora iz grafa
void delete_node(word *&words, int &num_nodes, connection *& connections, int &num_con, string name){
    int del_words_total = 0; int del_words_before = 0;
    connection* new_connections = make_array<connection>(num_con);
    int num_new_con = 0;

    for(int words_iterator = 0; words_iterator < num_nodes - 1; words_iterator++){
        for(int con_iterator = 0; con_iterator < words[words_iterator+1].id - words[words_iterator].id; con_iterator++){
            if(words[words_iterator].name == name){
                del_words_total += 1;
            }else if(connections[words[words_iterator].id + con_iterator].connection_name != name){
                new_connections[num_new_con] = connections[words[words_iterator].id + con_iterator];
                num_new_con += 1;
            }else{
                del_words_total += 1;
            }
        }
        words[words_iterator].id = words[words_iterator].id - del_words_before;
        del_words_before = del_words_total;
    }

    words[num_nodes-1].id = words[num_nodes-1].id - del_words_before;

    //menjanje duzine nizova
    num_nodes -= 1;
    num_con = num_new_con;

    word* new_words = make_array<word>(num_nodes);
    int j = 0;
    for(int i = 0; i < num_nodes+1; i++){
        if(words[i].name != name){
            new_words[j] = words[i];
            j++;
        }
    }

    delete_array(words);
    words = make_array<word>(num_nodes);
    for(int i = 0; i < num_nodes; i++){
        words[i] = new_words[i];
    }

    delete_array(connections);
    connections = make_array<connection>(num_con);

    for(int i = 0; i < num_con; i++){
        connections[i] = new_connections[i];
    }
    delete_array(new_connections);

}

//funkcija za dodavanje veze izmedju dve reci
void add_edge(word *&words, int &num_nodes, connection *& connections, int &num_con, string word1, string word2, float weight){
    num_con += 1;
    connection* new_connections = make_array<connection>(num_con);
    int j = 1; int insert_index = 0;
    for(int i = 0; i < num_nodes; i++){
        if(words[i].name == word1){
            j = 0;
            insert_index = words[i].id;
        }else if(j == 0){
            words[i].id += 1;
        }
    }
    for(int i = 0; i < num_con-1; i++){
        if(i == insert_index){
            connection new_con;
            new_con.connection_name = word2;
            new_con.weight = weight;
            new_connections[j] = new_con;
            j++;
        }
        new_connections[j] = connections[i];
        j++;
    }
    if(insert_index == num_con-1){
        connection new_con;
        new_con.connection_name = word2;
        new_con.weight = weight;
        new_connections[insert_index] = new_con;
    }

    delete_array(connections);
    connections = make_array<connection>(num_con);
    for(int i = 0; i < num_con; i++){
        connections[i] = new_connections[i];
    }

    delete_array(new_connections);
}

//funkcija za brisanje veze izmedju dve reci
void remove_edge(word *&words, int &num_nodes, connection *& connections, int &num_con, string word1, string word2){
    int index = -1; int last = 0;
    for(int i = 0; i < num_nodes; i++){
        if(words[i].name == word1){
            index = words[i].id;
            last = words[i+1].id;
        }else if(index != -1){
            words[i].id -= 1;
        }
    }

    for(int i = index; i < last; i++){
        if(connections[i].connection_name == word2){
            index = i;
        }
    }
    num_con -= 1;
    connection* new_connections = make_array<connection>(num_con);
    int j = 0;
    for(int i = 0; i < num_con; i++){
        if(i == index){
            j++;
        }
        new_connections[i] = connections[j];
        j++;
    }

    delete_array(connections);
    connections = make_array<connection>(num_con);
    for(int i = 0; i < num_con; i++){
        connections[i] = new_connections[i];
    }

    delete_array(new_connections);
}



//funkcija koja stampa dijkstra tablicu
void print_dijkstra(str_dijkstra* dijkstra_data, int num_words){
    for(int i = 0; i < num_words; i++){
        cout << dijkstra_data[i].vertex << ' ' << dijkstra_data[i].short_dist << ' ' << dijkstra_data[i].prev << ' ' << dijkstra_data[i].isvis << '\n';
    }
}

//funkcija koja vraca popunjenu tablicu nakon izvrsavanja dijkstre
str_dijkstra* dijkstra_data(word *&words, int &num_nodes, connection *& connections, int &num_con, string word_sim){
    num_nodes = num_nodes - 1;
    str_dijkstra* dijkstra = make_array<str_dijkstra>(num_nodes);
    for(int i = 0; i < num_nodes; i++){
        str_dijkstra a;
        a.vertex = words[i].name;
        a.short_dist = 0;
        a.prev = "";
        a.isvis = 0;
        if(a.vertex == word_sim){
            a.short_dist = 1;
        }
        dijkstra[i] = a;
    }

    float max_val; string node_name; int start; int end_of_iter;
    for(int i = 0; i < num_nodes; i++){
        max_val = -1;
        for(int j = 0; j < num_nodes; j++){
            if(dijkstra[j].short_dist > max_val && dijkstra[j].isvis == 0){
                max_val = dijkstra[j].short_dist;
                node_name = dijkstra[j].vertex;
            }
        }
        for(int j = 0; j < num_nodes; j++){
            if(words[j].name == node_name){
                start = words[j].id;
                end_of_iter = words[j+1].id;
            }
        }
        for(int j = start; j < end_of_iter; j++){
            float distance = connections[j].weight * max_val;
            for(int z = 0; z < num_nodes; z++){
                if(dijkstra[z].vertex == connections[j].connection_name && distance > dijkstra[z].short_dist && dijkstra[z].isvis == 0){
                    dijkstra[z].prev = node_name;
                    dijkstra[z].short_dist = distance;
                }
            }
        }
        for(int j = 0; j < num_nodes; j++){
            if(dijkstra[j].vertex == node_name){
                dijkstra[j].isvis = 1;
            }
        }
    }

    num_nodes += 1;

    return dijkstra;
}

//stampa k najslicnijih cvorova zadatom
void k_similar(str_dijkstra* dijkstra_data, int num_nodes, int k){
    int num_words = num_nodes-1;
    float maximum; float prev_maximum = 1; string node_name = "";
    int num_of_elem = 0;
    for(int i = 0; i < num_words; i++){
        maximum = 0;
        for(int j = 0; j < num_words; j++){
            if(dijkstra_data[j].short_dist > maximum && dijkstra_data[j].short_dist < prev_maximum){
                maximum = dijkstra_data[j].short_dist;
                node_name = dijkstra_data[j].vertex;
            }
        }
        if(num_of_elem < k && maximum != 0){
            cout << node_name << ' ' << maximum << '\n';
            num_of_elem += 1;
        }
        prev_maximum = maximum;
    }
}

//nalazi najkraci put izmedju dve zadate reci
void shortest_path(str_dijkstra* dijkstra_data, int num_nodes, string first_word, string second_word){
    string *inverted_path = make_array<string>(num_nodes);
    float *inverted_sim = make_array<float>(num_nodes);
    int num_path = 0;
    string comp_str = second_word;
    while(comp_str != first_word){
        int j = 0;
        while(dijkstra_data[j].vertex != comp_str){
            j++;
        }
        comp_str = dijkstra_data[j].prev;
        if(comp_str == ""){
            delete_array(inverted_path);
            delete_array(inverted_sim);
            return;
        }
        inverted_path[num_path] = dijkstra_data[j].vertex;
        inverted_sim[num_path] = dijkstra_data[j].short_dist;
        num_path += 1;
    }
    inverted_path[num_path] = first_word;
    inverted_sim[num_path] = 1;
    while( num_path != 0 ){
        cout << inverted_path[num_path] << ' ' << '[' << inverted_sim[num_path-1]/inverted_sim[num_path] << ']' << " -> ";
        num_path -= 1;
    }
    cout << inverted_path[num_path];

    delete_array(inverted_path);
    delete_array(inverted_sim);
}

//trazi sve jake (dvostruke) konekcije zadatog cvora
void strong_connection(word *&words, int &num_nodes, connection *& connections, int &num_con, str_dijkstra* dijkstra, string dij_word){
    int num_nodes_less =  num_nodes - 1;
    int is_any = 0;
    for(int i = 0; i < num_nodes_less; i++){
        if(dijkstra[i].short_dist != 0 && dijkstra[i].vertex != dij_word){
            str_dijkstra* dijkstra_data_second = dijkstra_data(words, num_nodes, connections, num_con, dijkstra[i].vertex);
            for(int j = 0; j < num_nodes_less; j++){
                if(dijkstra_data_second[j].short_dist != 0 && dijkstra_data_second[j].vertex == dij_word){
                    cout << dijkstra[i].vertex << ' ';
                    is_any = 1;
                }
            }
            delete_array(dijkstra_data_second);
        }
    }
    if(is_any){
        cout << dij_word << '\n';
    }
}



int main(){

    string file_name; // C:/Users/Zephyrus/Desktop/graf2.txt ili C:/Users/Zephyrus/Desktop/graf1.txt
    string new_node; string word1; string word2;
    representation repr;
    int task; int is_open = 0; int br_k;
    float weight;

    cout << "Zdravo! Dobro dosli u program GRAF!\n";

    while(1){
        cout << "Unesite pitanju na kojoj se nalazi fajl za ucitavanje grafa:\n\n";
        cin >> file_name;
        ifstream myfile;
        myfile.open(file_name);
        if(myfile){
            is_open = 1;
            repr = load_graph(file_name);
            myfile.close();
        }else{
            cout << "\nFajl na datoj putanji ne postoji.\n";
        }
        while(is_open) {
            cout << "\nIzaberite jednu od sledecih opcija:"
                << "\n1. Ispis reprezentacije grafa\n"
                << "2. Dodavanje cvora u graf\n"
                << "3. Uklanjanje cvora iz grafa\n"
                << "4. Dodavanje grane izmedju dva cvora u grafu\n"
                << "5. Uklanjanje grane izmedju dva cvora u grafu\n"
                << "6. Nalazenje k semanticki najslicnijih reci zadatoj\n"
                << "7. Ispis svih reci na najkracem putu od jedne do druge zadate\n"
                << "8. Nalazenje svih reci koje su jako povezane sa zadatom\n\n";

            scanf("%d", &task);
            cout << '\n';
            if(task == 1){
                cout << "Ispis svih cvorova i pokazivaca na njihove konekcije:\n";
                print_words(repr.words, repr.num_words);
                cout << "Ispis svih konekcija:\n";
                print_connections(repr.connections, repr.num_con);
            }else if(task == 2){
                cout << "Unesite ime novog cvora:\n";
                cin >> new_node;
                if(is_node_in(repr.words, repr.num_words, new_node)){
                    cout << "\nCvor vec postoji u grafu.\n";
                }else{
                    add_node(repr.words, repr.num_words, repr.num_con, new_node);
                    cout << "\nCvor je uspesno dodat u graf.\n";
                }
            }else if(task == 3){
                cout << "Unesite ime cvora koji zelite da uklonite:\n";
                cin >> new_node;
                if(is_node_in(repr.words, repr.num_words, new_node)){
                    delete_node(repr.words, repr.num_words, repr.connections,repr.num_con, new_node);
                    cout << "\nCvor je uspesno izbacen iz grafa.\n";
                }else{
                    cout << "\nDati cvor ne postoji u grafu.\n";
                }
            }else if(task == 4){
                cout << "Unesite ime prvog cvora:\n";
                cin >> word1;
                cout<< "\nUnesite ime drugog cvora:\n";
                cin >> word2;
                if(is_node_in(repr.words, repr.num_words, word1) && is_node_in(repr.words, repr.num_words, word2)){
                    cout << "\nUnesite tezinu veze:\n";
                    scanf("%f", &weight);
                    if(weight > 0 && weight < 1){
                        add_edge(repr.words, repr.num_words, repr.connections, repr.num_con, word1, word2, weight);
                        cout << "\nVeza je uspesno dodata u graf.\n";
                    }else{
                        cout << "\nPogresno uneta tezina veze.\n";
                    }
                }else{
                    cout << "\nJedan ili oba cvora ne postoje unutar grafa.\n";
                }

            }else if(task == 5){
                cout << "Unesite ime prvog cvora:\n";
                cin >> word1;
                cout<< "\nUnesite ime drugog cvora:\n";
                cin >> word2;
                if(is_node_in(repr.words, repr.num_words, word1) && is_node_in(repr.words, repr.num_words, word2)){
                    if(is_connection_in(repr.words, repr.num_words, repr.connections, repr.num_con, word1, word2)){
                        remove_edge(repr.words, repr.num_words, repr.connections, repr.num_con, word1, word2);
                        cout << "\nVeza je uspesno uklonjena iz grafa.\n";
                    }else{
                        cout << "\nNe postoji veza izmedju ta dva cvora.\n";
                    }
                }else{
                    cout << "\nJedan ili oba cvora ne postoje unutar grafa.\n";
                }
            }else if(task == 6){
                cout << "Unesite ime cvora:\n";
                cin >> new_node;
                if(is_node_in(repr.words, repr.num_words, new_node)){
                    cout << "\nUnesite broj k (broj semanticki najblizih reci):\n";
                    cin >> br_k;
                    str_dijkstra* dij_data = dijkstra_data(repr.words, repr.num_words, repr.connections, repr.num_con, new_node);
                    k_similar(dij_data, repr.num_words, br_k);
                }else{
                    cout << "\nCvor ne postoji unutar grafa.\n";
                }
                //C:/Users/Zephyrus/Desktop/graf1.txt
            }else if(task == 7){
                cout << "Unesite ime prvog cvora:\n";
                cin >> word1;
                cout<< "\nUnesite ime drugog cvora:\n";
                cin >> word2;
                if(is_node_in(repr.words, repr.num_words, word1) && is_node_in(repr.words, repr.num_words, word2)){
                    str_dijkstra* dij_data = dijkstra_data(repr.words, repr.num_words, repr.connections, repr.num_con, word1);
                    shortest_path(dij_data, repr.num_words, word1, word2);
                }else{
                    cout << "\nJedan ili oba cvora ne postoje unutar grafa.\n";
                }
            }else if(task == 8){
                cout << "Unesite rec:\n";
                cin >> new_node;
                if(is_node_in(repr.words, repr.num_words, new_node)){
                    str_dijkstra* dij_data = dijkstra_data(repr.words, repr.num_words, repr.connections, repr.num_con, new_node);
                    strong_connection(repr.words, repr.num_words, repr.connections, repr.num_con, dij_data, new_node);
                }else{
                    cout << "\nCvor ne postoji unutar grafa.\n";
                }
            }else{
                cout << "Uneti broj opcije je pogresan.\n";
            }
        }
    }

    return 0;
}
