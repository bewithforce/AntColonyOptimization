#include <iostream>
#include <vector>
#include <cmath>

using namespace std;


#define ALPHA	1			// вес фермента
#define BETTA	2			// коэффициент эвристики

#define tau0 10

#define T_MAX 500			// время жизни колонии
#define Q 10			// количество
#define RHO	0.6			// коэффициент испарения феромона

class Edge;

class City{
private:
    int num;
public:
    City(int num){
        this->num = num;
    }
    vector<Edge*> edges;

    friend ostream &operator<<(ostream &os, const City &city) {
        os << city.num;
        return os;
    }
};

struct Edge{
    int length;
    double pheromone;
    City *first, *second;
    Edge(int length, int pheromone){
        this->length = length;
        this->pheromone = pheromone;
    }
    City* getAnother(const City& city) const{
        if(&city == first){
            return second;
        }
        return first;
    }
};

struct Ant{
    vector<City*> path;
    vector<Edge*> pheromonePath;
    City *curCity;
    City *startCity;
    int pathLength = 0;
    Ant(City& city){
        curCity = &city;
        startCity = &city;
    }
};


double probability(const Edge& edge, const Ant& ant) {
    // если вершина уже посещена, возвращаем 0
    for (City *city : ant.path)
        if (edge.getAnother(*(ant.curCity)) == city)
            return 0;

    double sum = 0.0;
    // считаем сумму в знаменателе
    for(Edge *edge1 : ant.curCity->edges){
        int flag = 1;
        for (City *city : ant.path)
            if (edge1->getAnother(*(ant.curCity)) == city)
                flag = 0;
        if(flag)
            sum += (pow(edge1->pheromone, ALPHA) * pow((double)1/(double)edge1->length, BETTA));
    }
    // возвращаем значение вероятности
    return pow(edge.pheromone, ALPHA) * pow((double)1/(double)edge.length, BETTA) / sum;
}

Edge* getEdge(const Ant& ant){
    double p_max = 0;
    Edge* resultEdge = nullptr;
    for (Edge *edge : ant.curCity->edges) {
        // Проверка вероятности перехода по ребру edge
        double p = probability(*edge, ant);
        if (p > p_max) {
            p_max = p;
            resultEdge = edge;
        }
    }
    if(resultEdge == nullptr){
        for (Edge *edge : ant.curCity->edges) {
            // Проверка вероятности перехода по ребру edge
            if(edge->getAnother(*ant.curCity) == ant.startCity)
                return edge;
        }
    }
    return resultEdge;
}

Ant* aco(int **d, int n){
    Ant *bestAnt = nullptr;
    vector<City*> cities;
    vector<Edge*> edges;
    vector<Ant*> ants;
    for (int i = 0; i < n; ++i) {
        City *temp = new City(i + 1);
        cities.push_back(temp);
    }
    int M = 0.0222 * (n * n) - 0.1333*n + 8;
    cout << M << endl;
    for (int i = 0; i < M && i < n; ++i) {
        Ant *ant = new Ant(*cities[i]);
        ants.push_back(ant);
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if(d[i][j]) {
                Edge *edge = new Edge(d[i][j], tau0);
                edge->first = cities[i];
                edge->second = cities[j];
                edges.push_back(edge);
                cities[i]->edges.push_back(edge);
                cities[j]->edges.push_back(edge);
            }
        }
    }
    for(int t = 0; t < T_MAX; t++) {
        for (Ant *ant : ants) {
            ant->path.push_back(ant->curCity);
            do {
                Edge *edge = getEdge(*ant);
                ant->pathLength += edge->length;
                City *city = edge->getAnother(*(ant->curCity));
                ant->pheromonePath.push_back(edge);
                ant->path.push_back(city);
                ant->curCity = city;
            } while (ant->curCity != ant->startCity);

            for (Edge *pathEdge : ant->pheromonePath) {
                pathEdge->pheromone += (double)Q/(double)ant->pathLength;
            }
            // проверка на лучшее решение
            if(bestAnt == nullptr){
                bestAnt = new Ant(*(ant->startCity));
                bestAnt->pathLength = ant->pathLength;
                for (City *city : ant->path)
                    bestAnt->path.push_back(city);
                for (Edge *edge : ant->pheromonePath)
                    bestAnt->pheromonePath.push_back(edge);
            } else if (ant->pathLength < bestAnt->pathLength) {
                delete bestAnt;
                bestAnt = new Ant(*(ant->startCity));
                bestAnt->pathLength = ant->pathLength;
                for (City *city : ant->path)
                    bestAnt->path.push_back(city);
                for (Edge *edge : ant->pheromonePath)
                    bestAnt->pheromonePath.push_back(edge);
            }

            ant->pathLength = 0;
            ant->path.clear();
            ant->pheromonePath.clear();
        }
        for(Edge *edge : edges){
            edge->pheromone *= (1 - RHO);
        }
    }
    return bestAnt;
}

int main() {
    int n;
    cout<<"input number of cities"<<endl;
    cin >> n;
    cout<<"input matrix of distances"<<endl;
    int **d = new int *[n];
    for (int i = 0; i < n; ++i) {
        d[i] = new int[n];
        for (int j = 0; j < n; ++j) {
            cin >> d[i][j];
        }
    }
    Ant *best = aco(d, n);

    City *curCity = best->startCity;
    for (int i = 0; i < best->path.size() - 1; i++){
        cout<< *curCity << " -> ";
        curCity = best->path[i + 1];
    }
    cout<< *curCity << endl;
    cout << best->pathLength;
}
