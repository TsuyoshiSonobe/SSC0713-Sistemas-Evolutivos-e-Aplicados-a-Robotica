#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define nGer 500         // Numero de geracoes
#define nInd 50         // Numero de individuos em uma geracao  
//#define nGene 100         // Numero de genes por individuo
#define taxMut 5        // Taxa de mutacao

int main(){  

    Mat img = imread("face.png", IMREAD_COLOR);

    Mat imgGrayScale;
    cvtColor(img, imgGrayScale, COLOR_BGR2GRAY);

    Mat black;
    inRange(imgGrayScale, 127, 255, black);  

    // Transformando Mat em array
    vector<uchar> array;
    if (black.isContinuous()) {
        array.assign(black.data, black.data + black.total()*black.channels());
    }
    else {
        for (int i = 0; i < black.rows; ++i) {
            array.insert(array.end(), black.ptr<uchar>(i), black.ptr<uchar>(i)+black.cols*black.channels());
        }
    }

    int referencia[array.size()];
    copy(array.begin(), array.end(), referencia);
    
    for(int i = 0; i < array.size(); i++){
        if(referencia[i] == 255){
            referencia[i] = 1;
        }
    } 

    // Algoritmo Genetico
    const int nGene = array.size();                 // numero de genes por individuo
    int fitMedioGer = 0, fitGer = 0, fitInd = 0;    // fitness medio e total da geracao e fitness de individuo
    int fitMInd = -1, mInd = -1;                    // fitness e numero do melhor individuo
    int fitPInd = 100, pInd = -1;                   // fitness e numero do pior individuo
    int pop[nInd][nGene];                           // matriz de individuos de uma geracao (populacao)
    uint8_t imgFinal[10][10];                       // matriz que armazenara a imagem final

    // Criacao da populacao inicial
    srand(time(NULL));    
    for(int j = 0; j < nInd; j++){          
        for(int i = 0; i < nGene; i++){
            pop[j][i] = rand() % 2;
        }
    }
    
    // Passando pelas geracoes
    for(int g = 0; g < nGer; g++){    

        // Imprimindo a populacao da geracao
        /*cout << "Geração " << g << endl;
        for(int j = 0; j < nInd; j++){
            cout << "Individuo " << j << endl;
            for(int i = 0; i < nGene; i++){
                cout << pop[j][i] << " ";
            }
            cout << endl;
        }*/

        // Avaliacao e selecao da populacao da geracao
        for(int j = 0; j < nInd; j++){           // passando por cada individuo
            for(int i = 0; i < nGene; i++){      // passando por cada gene do individuo
                if(pop[j][i] == referencia[i]){
                    fitGer++;
                    fitInd++;
                }
            }
        //    cout << "Fitness do individuo " << j << ": " << fitInd << endl;
            if(fitInd > fitMInd){
                fitMInd = fitInd;
                mInd = j;
            }
            if(fitInd < fitPInd){
                fitPInd = fitInd;
                pInd = j;
            }
            fitInd = 0;
        }

        // Croosover
        int corte = rand()%nGene;
        for(int j = 0; j < nInd; j++){
            for(int i = 0; i < corte; i++){
                pop[j][i] = pop[mInd][i];
            }
        }
        
        // Mutacao
        if (rand()%11 < 7) {                     // probabilidade de mutacao
            for(int j = 0; j < nInd; j++){
                if(j != mInd){                          // nao muta o melhor individuo
                    int pontoMutacao = rand()%nGene;    // muta um gene aleatorio
                    if(pop[j][pontoMutacao] == 1){
                        pop[j][pontoMutacao] = 0;
                    }
                    else{
                        pop[j][pontoMutacao] = 1;
                    }
                }
            }
        }

        // Trocando o pior individuo pelo melhor
        for(int i = 0; i < nGene; i++){
            pop[pInd][i] = pop[mInd][i];
        }

        // Resultados da geracao
        //cout << "Numero do melhor individuo da geracao: " << mInd << " -> Fitness: " << fitMInd << endl;
        fitMedioGer = (fitGer/nInd);       
        //cout << "Fitness medio da geração: " << fitMedioGer << endl;
        fitGer = 0;
        
        // Encerra antes caso chegue ao caso ideal
        if(fitMedioGer == nGene)
            break;
    }

    // Transforma o array binario em matriz de cores (branco e preto, por enquanto)
    int k = 0;
    for(int j = 0; j < 10; j++){
        for(int i = 0; i < 10; i++){
            imgFinal[j][i] = pop[mInd][k];
            k++;
            if(imgFinal[j][i] == 1)
                imgFinal[j][i] = 255;
        }
    }

    // Transforma a matriz em imagem e mostra
    Mat finalImg = Mat(10, 10, CV_8U, &imgFinal);
    namedWindow("Imagem Final", WINDOW_NORMAL);
    imshow("Imagem Final", finalImg);
    waitKey(0);
    destroyAllWindows();

    // Visualizar imagem original
   /* namedWindow("Original", WINDOW_NORMAL);
    namedWindow("Gray Scale", WINDOW_NORMAL);
    namedWindow("Black", WINDOW_NORMAL);
    imshow("Original", img);
    imshow("Gray Scale", imgGrayScale);
    imshow("Black", black);
    waitKey(0);
    destroyWindow("Original");
    destroyWindow("Gray Scale");
    destroyWindow("Black");*/

    return 0;
}