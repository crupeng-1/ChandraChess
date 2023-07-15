#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "bits.h"
#include "board.h"
#include "evaluation.h"
#include "hashKey.h"
#include "moveGenerator.h"
#include "search.h"
#include "state.h"
#include "table.h"
#include "updateState.h"
double timeManagementTable[300][2] = {{38.38403209089165, 37.91762564782931}, {37.4550091489032, 36.93054925979915}, {36.46283709606776, 35.94010541188089}, {35.47487372921934, 34.95528847905075}, {34.49267517455615, 33.97783712885328}, {33.52249476342535, 33.016102398952675}, {32.567923773591794, 32.07054054362788}, {31.631592489030087, 31.146029412227794}, {30.71791584471641, 30.245992966508773}, {29.826734372612698, 29.378751520170116}, {28.98988385531598, 28.545234744597657}, {28.152221783072665, 27.71445657677503}, {27.32910878095436, 26.89941824601143}, {26.516167774905277, 26.095060574506125}, {25.72000118950264, 25.31051821707511}, {24.950740353434274, 24.55125825200767}, {24.19107166081612, 23.804949425447216}, {23.4601495988616, 23.08579746371923}, {22.741277122702662, 22.382947952008845}, {22.046562506592792, 21.7051594827463}, {21.397505866384098, 21.07341583427055}, {20.76100776888646, 20.449628351342184}, {20.147039829488225, 19.854656891667144}, {19.563615232708354, 19.287856464859065}, {19.00996781671202, 18.747881448554704}, {18.494865486050305, 18.248405064208875}, {17.996018379196876, 17.762064730476332}, {17.520020953370803, 17.299526694470398}, {17.068527772878667, 16.857728373397023}, {16.638173827939145, 16.448772283971742}, {16.26594939960999, 16.084797541249568}, {15.888691286112065, 15.717309504785527}, {15.527017544180458, 15.358070967089896}, {15.175307939203467, 15.010758763324327}, {14.832788322101639, 14.677154492333724}, {14.512721076844437, 14.359945229948135}, {14.197534346027727, 14.048906100603666}, {13.88698769482232, 13.743511194745793}, {13.587446557822664, 13.452920736087645}, {13.322356978709395, 13.252886536858702}, {13.333885314387738, 13.445085991837166}, {13.442071363789958, 13.432418460679717}, {13.359975363002569, 13.298033980870708}, {13.200269067631671, 13.118497758253211}, {13.007874243399272, 12.916803188981275}, {12.810509924879359, 12.721240855550901}, {12.612486094036718, 12.523513729906648}, {12.411191752680345, 12.32037269812228}, {12.212990084665629, 12.123510907836213}, {12.012055455878256, 11.928355076857427}, {11.848544154483253, 11.77580012659425}, {11.681997631565054, 11.602267653274968}, {11.501056560501691, 11.422693045842848}, {11.323210864242505, 11.251396678583532}, {11.152983761405048, 11.081649054039413}, {10.985658305284623, 10.915248670862185}, {10.842759200224087, 10.799598155697247}, {10.724771820013952, 10.67399202526009}, {10.597985306889607, 10.55244587340794}, {10.490314315885445, 10.492386740833927}, {10.789249082386746, 10.823146368776873}, {10.815228341777015, 10.794536781015466}, {10.758881081102553, 10.73138495982156}, {10.699530945817056, 10.67770289398464}, {10.643289637803404, 10.619889165029864}, {10.594707550141763, 10.5864133881062}, {10.562075183798031, 10.545194278990211}, {10.514624194429969, 10.50568451129225}, {10.489419843049328, 10.478603230672995}, {10.467280329553974, 10.4702678535138}, {10.468345668039722, 10.483271542725506}, {10.467664370813374, 10.46978352225876}, {10.492607639470599, 10.537137550988824}, {10.543612765165612, 10.576151715007233}, {10.582626189677852, 10.625204026115343}, {10.678239875837093, 10.713615236711027}, {10.719094685575133, 10.731150063557905}, {10.737111408074272, 10.753276037334812}, {10.774135180357213, 10.816426507061307}, {10.834433229813664, 10.89803163044715}, {11.21364810033198, 11.297896103896104}, {11.345242973039417, 11.374157271136356}, {11.390003308171183, 11.440883348625304}, {11.470053120849933, 11.509259582185331}, {11.528349479071467, 11.538655944859276}, {11.537657239003286, 11.569086552608349}, {11.599973540306932, 11.656915016922435}, {11.692042027434576, 11.728098372365936}, {11.807423721529867, 11.86635814662227}, {11.904088515472457, 11.970220702517874}, {12.08062930186824, 12.136735676456427}, {12.14942859196435, 12.185204695966286}, {12.17024949003609, 12.209726443768997}, {12.175132546605097, 12.194459338695264}, {12.190485074626865, 12.200837635141717}, {12.23852275045909, 12.26002132196162}, {12.263163753757096, 12.267015098722416}, {12.238659731462441, 12.223916330645162}, {12.213750492967003, 12.200109574030956}, {12.146233803217998, 12.215715566178668}, {12.419482785974932, 12.563253012048193}, {12.603221852565225, 12.681592952835382}, {12.682488038277512, 12.733146629325866}, {12.73279098873592, 12.75364209610785}, {12.847647327546825, 12.928725185362353}, {12.989982469321312, 13.035546262415055}, {13.038126361655774, 13.048455653159536}, {13.07810650887574, 13.123148148148148}, {13.130910260533934, 13.220801077078425}, {13.200910045502274, 13.20290909090909}, {13.228842504743833, 13.206611570247935}, {13.309789343246592, 13.386706948640484}, {13.46070460704607, 13.512241054613936}, {13.457560975609756, 13.579969340827798}, {13.620266666666666, 13.73828125}, {13.870457209847597, 13.885036496350365}, {13.800754242614708, 13.836054866100588}, {13.845003399048267, 13.849189570119803}, {13.954948301329395, 13.982361963190185}, {13.976892430278884, 14.1075}, {14.437943262411348, 14.604456824512535}, {14.702230843840931, 14.666666666666666}, {14.59400826446281, 14.712284482758621}, {14.71923937360179, 14.762180974477959}, {14.830713422007255, 14.81023720349563}, {14.835278858625163, 14.808567603748326}, {15.0028129395218, 15.058394160583942}, {15.130699088145896, 15.141509433962264}, {15.168026101141924, 15.192567567567568}, {15.130662020905923, 15.193490054249548}, {15.332703213610586, 15.450787401574804}, {15.25553319919517, 15.230290456431534}, {15.368763557483732, 15.242222222222223}, {15.262672811059907, 15.5181598062954}, {15.359801488833748, 15.73753280839895}, {15.64054054054054, 15.861581920903955}, {15.885630498533724, 16.08868501529052}, {16.114285714285714, 16.124183006535947}, {16.24914675767918, 16.829090909090908}, {16.924242424242426, 16.872093023255815}, {16.951612903225808, 17.0625}, {17.125541125541126, 17.057522123893804}, {17.16589861751152, 17.037558685446008}, {16.946859903381643, 16.745098039215687}, {16.92820512820513, 16.729166666666668}, {16.52127659574468, 16.149732620320854}, {15.858695652173912, 16.09659090909091}, {15.622857142857143, 15.629411764705882}, {15.699386503067485, 15.4472049689441}, {15.558441558441558, 15.716216216216216}, {16.014285714285716, 16.014705882352942}, {15.924242424242424, 16.336}, {16.016260162601625, 16.10924369747899}, {15.922413793103448, 16.345454545454544}, {16.330188679245282, 16.54901960784314}, {16.927083333333332, 17.23913043478261}, {17.375, 17.372093023255815}, {17.154761904761905, 16.761904761904763}, {16.349397590361445, 16.146341463414632}, {16.545454545454547, 16.56}, {16.625, 17.16176470588235}, {16.791044776119403, 16.907692307692308}, {16.793650793650794, 16.950819672131146}, {18.09090909090909, 19.07843137254902}, {19.583333333333332, 20.954545454545453}, {20.74418604651163, 20.41860465116279}, {20.214285714285715, 19.88095238095238}, {20.175, 20.86842105263158}, {20.18421052631579, 19.86842105263158}, {20.25, 21.08823529411765}, {21.0, 21.34375}, {21.29032258064516, 21.7}, {22.464285714285715, 22.178571428571427}, {21.464285714285715, 21.962962962962962}, {21.22222222222222, 20.962962962962962}, {20.22222222222222, 21.56}, {20.76, 20.56}, {19.76, 19.56}, {20.391304347826086, 21.09090909090909}, {21.238095238095237, 23.263157894736842}, {26.5625, 26.4375}, {25.5625, 27.133333333333333}, {26.2, 26.133333333333333}, {29.076923076923077, 29.0}, {30.416666666666668, 30.333333333333332}, {29.416666666666668, 29.333333333333332}, {28.416666666666668, 28.333333333333332}, {27.416666666666668, 27.333333333333332}, {26.416666666666668, 26.333333333333332}, {25.416666666666668, 25.333333333333332}, {29.3, 29.2}, {31.444444444444443, 31.333333333333332}, {34.25, 34.125}, {33.25, 33.125}, {36.857142857142854, 36.714285714285715}, {35.857142857142854, 35.714285714285715}, {34.857142857142854, 34.714285714285715}, {33.857142857142854, 33.714285714285715}, {32.857142857142854, 32.714285714285715}, {44.6, 44.4}, {43.6, 54.25}, {53.25, 53.25}, {52.25, 52.25}, {51.25, 51.25}, {50.25, 50.25}, {49.25, 49.25}, {48.25, 48.25}, {47.25, 47.25}, {46.25, 46.25}, {45.25, 45.25}, {44.25, 44.25}, {57.666666666666664, 57.666666666666664}, {56.666666666666664, 56.666666666666664}, {55.666666666666664, 55.666666666666664}, {54.666666666666664, 54.666666666666664}, {53.666666666666664, 53.666666666666664}, {52.666666666666664, 52.666666666666664}, {51.666666666666664, 51.666666666666664}, {50.666666666666664, 50.666666666666664}, {49.666666666666664, 49.666666666666664}, {48.666666666666664, 48.666666666666664}, {47.666666666666664, 47.666666666666664}, {46.666666666666664, 46.666666666666664}, {45.666666666666664, 45.666666666666664}, {44.666666666666664, 44.666666666666664}, {43.666666666666664, 43.666666666666664}, {42.666666666666664, 42.666666666666664}, {62.5, 62.5}, {61.5, 61.5}, {60.5, 60.5}, {59.5, 59.5}, {58.5, 58.5}, {57.5, 57.5}, {56.5, 56.5}, {55.5, 55.5}, {54.5, 54.5}, {53.5, 53.5}, {52.5, 52.5}, {51.5, 51.5}, {50.5, 50.5}, {49.5, 49.5}, {48.5, 48.5}, {47.5, 47.5}, {46.5, 46.5}, {45.5, 45.5}, {44.5, 44.5}, {43.5, 43.5}, {42.5, 42.5}, {41.5, 41.5}, {40.5, 40.5}, {39.5, 39.5}, {38.5, 38.5}, {37.5, 37.5}, {36.5, 36.5}, {35.5, 35.5}, {34.5, 34.5}, {33.5, 33.5}, {32.5, 32.5}, {31.5, 31.5}, {30.5, 30.5}, {29.5, 29.5}, {28.5, 28.5}, {27.5, 27.5}, {26.5, 26.5}, {25.5, 25.5}, {24.5, 24.5}, {23.5, 23.5}, {22.5, 22.5}, {21.5, 21.5}, {20.5, 20.5}, {19.5, 19.5}, {18.5, 18.5}, {17.5, 17.5}, {16.5, 16.5}, {15.5, 15.5}, {14.5, 14.5}, {13.5, 13.5}, {12.5, 12.5}, {11.5, 11.5}, {21.0, 21.0}, {20.0, 20.0}, {19.0, 19.0}, {18.0, 18.0}, {17.0, 17.0}, {16.0, 16.0}, {15.0, 15.0}, {14.0, 14.0}, {13.0, 13.0}, {12.0, 12.0}, {11.0, 11.0}, {10.0, 10.0}, {9.0, 9.0}, {8.0, 8.0}, {7.0, 7.0}, {6.0, 6.0}, {5.0, 5.0}, {4.0, 4.0}, {3.0, 3.0}, {2.0, 2.0}, {1.0, 1.0}};
std::chrono::steady_clock::time_point startTime;
int boardSize = sizeof(board);
int numberOfThreads = 1;
int depthLimit = 63;
int timeToSearch;
int minimumTimeRemaining = 50;
volatile bool isInterruptedByGui;
volatile bool isCurrentlySearching;
void checkUp(board& inputBoard) {
  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
  int timeElapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
  if (timeElapsed > timeToSearch || isInterruptedByGui) inputBoard.isSearchStopped = true;
}
int quiescence(board& inputBoard, int alpha, int beta) {
  inputBoard.nodes++;
  if ((inputBoard.nodes & 2047) == 0) checkUp(inputBoard);
  int standingPat = evaluate(inputBoard);
  if (inputBoard.distanceToRoot == 63) return standingPat;
  if (standingPat >= beta) return beta;
  if (alpha < standingPat) alpha = standingPat;
  movesContainer moves;
  moves.numberOfMoves = 0;
  generateMoves(inputBoard, moves, true);
  orderMoves(inputBoard, moves);
  int bestScore = -20000;
  for (int i = 0; i < moves.numberOfMoves; i++) {
    moveEntry& move = moves.moveList[i];
    // Score for losing capture.
    // Prune these moves.
    if (move.score < 200) break;
    makeMove(inputBoard, move.move);
    int score = -quiescence(inputBoard, -beta, -alpha);
    takeMove(inputBoard, move.move);
    if (inputBoard.isSearchStopped) return 0;
    if (score > alpha) {
      alpha = score;
      if (score >= beta) return beta;
    }
  }
  return alpha;
}
int negamax(board& inputBoard, int depth, int alpha, int beta) {
  if ((inputBoard.nodes & 2047) == 0) checkUp(inputBoard);
  bool isSideToPlayInCheck = isSquareOfSideToPlayAttacked(inputBoard, msbPosition(inputBoard.bitboards[6 * inputBoard.sideToPlay]));
  if (isSideToPlayInCheck) depth++;
  movesContainer moves;
  moves.numberOfMoves = 0;
  generateMoves(inputBoard, moves, false);
  if (isSideToPlayInCheck && moves.numberOfMoves == 0) return -20000 + inputBoard.distanceToRoot;
  if (moves.numberOfMoves == 0 || inputBoard.halfMoveClock == 100 || isRepetition(inputBoard)) return 0;
  if (depth == 0 || inputBoard.distanceToRoot == 63) return quiescence(inputBoard, alpha, beta);
  inputBoard.nodes++;
  int score;
  if (probeTableEntry(inputBoard, depth, alpha, beta, score)) return score;
  if (depth >= 3 && !isSideToPlayInCheck && inputBoard.gamePhase >= 12) {
    makeNullMove(inputBoard);
    int score = -negamax(inputBoard, depth - 3, -beta, -beta + 1);
    takeNullMove(inputBoard);
    if (inputBoard.isSearchStopped) return 0;
    if (score >= beta) return beta;
  }
  int bestMove = 0;
  bool isPrincipleVariationNode = beta - alpha > 1;
  bool isDoingFullSearch = true;
  std::vector<int> deferredMoveIndices;
  orderMoves(inputBoard, moves);
  for (int i = 0; i < 2; i++) {
    // When i = 0 then we are doing normal search and deferring moves.
    // When i = 1 then we are doing deferred search.
    for (int j = 0; j < moves.numberOfMoves; j++) {
      if (i == 1) {
        bool isDeferringMove = std::find(deferredMoveIndices.begin(), deferredMoveIndices.end(), j) != deferredMoveIndices.end();
        if (!isDeferringMove) continue;
      }
      int move = moves.moveList[j].move;
      int from = move & 0x3f;
      int to = (move & 0xfc0) >> 6;
      int promotion = (move & 0x3000) >> 12;
      int type = (move & 0xc000) >> 14;
      bool isTacticalMove = type == 1 || type == 2 || (inputBoard.bitboards[14] & bits[to]) != 0ull;
      bool isStartingSearch = i == 0 && j != 0;
      makeMove(inputBoard, move);
      bool isMoveCausingCheck = isSquareOfSideToPlayAttacked(inputBoard, msbPosition(inputBoard.bitboards[6 * inputBoard.sideToPlay]));
      bool isDeferredBySyncronisation = i == 0 && j != 0 && depth >= 3 && syncronisationTable[inputBoard.currentKey & 65535] == inputBoard.currentKey;
      if (isDeferredBySyncronisation) {
        deferredMoveIndices.push_back(j);
        takeMove(inputBoard, move);
        continue;
      }
      bool isUsingLateMoveReduction = !isPrincipleVariationNode && depth >= 3 && j >= 3 && !isTacticalMove && !isSideToPlayInCheck && !isMoveCausingCheck;
      int score;
      if (isDoingFullSearch) {
        if (isUsingLateMoveReduction) {
          if (isStartingSearch) startingSearch(inputBoard, depth);
          score = -negamax(inputBoard, depth - 2, -beta, -alpha);
          if (isStartingSearch) endingSearch(inputBoard, depth);
          if (score > alpha) {
            score = -negamax(inputBoard, depth - 1, -beta, -alpha);
          }
        } else {
          score = -negamax(inputBoard, depth - 1, -beta, -alpha);
        }
      } else {
        if (isStartingSearch) startingSearch(inputBoard, depth);
        score = isUsingLateMoveReduction ? -negamax(inputBoard, depth - 2, -alpha - 1, -alpha) : -negamax(inputBoard, depth - 1, -alpha - 1, -alpha);
        if (isStartingSearch) endingSearch(inputBoard, depth);
        if (score > alpha) {
          score = -negamax(inputBoard, depth - 1, -beta, -alpha);
        }
      }
      takeMove(inputBoard, move);
      if (inputBoard.isSearchStopped) return 0;
      // Update alpha.
      if (score > alpha) {
        bestMove = move;
        alpha = score;
        isDoingFullSearch = false;
        // Check failed high cutoff.
        if (score >= beta) {
          // We are atleast as good as beta in this position.
          insertToTable(inputBoard, move, depth, beta, lowerBound);
          if (!isTacticalMove) {
            if (inputBoard.killersTable[inputBoard.distanceToRoot][0] != move && inputBoard.killersTable[inputBoard.distanceToRoot][1] != move && inputBoard.killersTable[inputBoard.distanceToRoot][2] != move) {
              inputBoard.killersTable[inputBoard.distanceToRoot][2] = inputBoard.killersTable[inputBoard.distanceToRoot][1];
              inputBoard.killersTable[inputBoard.distanceToRoot][1] = inputBoard.killersTable[inputBoard.distanceToRoot][0];
              inputBoard.killersTable[inputBoard.distanceToRoot][0] = move;
            }
            inputBoard.historyTable[inputBoard.sideToPlay][from][to] += depth * depth;
          }
          return beta;
        }
      }
    }
  }
  insertToTable(inputBoard, bestMove, depth, alpha, isDoingFullSearch ? upperBound : exact);
  return alpha;
}
void searchPosition(board& inputBoard) {
  board* workerDatas = (board*)malloc(numberOfThreads * boardSize);
  for (int i = 0; i < numberOfThreads; i++) workerDatas[i] = inputBoard;
  std::vector<int> principalVariations[numberOfThreads];
  board& firstWorkerData = workerDatas[0];
  std::vector<int> principalVariation;
  int threadsToSpawn = numberOfThreads - 1;
  int currentDepth = 1;
  while (true) {
    std::thread workerThreads[threadsToSpawn];
    for (int i = 0; i < threadsToSpawn; i++) {
      std::vector<int>& currentPrincipalVariation = principalVariations[i + 1];
      currentPrincipalVariation.clear();
      workerThreads[i] = std::thread(negamax, std::ref(workerDatas[i + 1]), currentDepth, -20000, 20000);
    }
    int score = negamax(firstWorkerData, currentDepth, -20000, 20000);
    // Stop all the threads once main thread has finished.
    for (int i = 0; i < threadsToSpawn; i++) {
      workerDatas[i + 1].isSearchStopped = true;
      if (workerThreads[i].joinable()) workerThreads[i].join();
    }
    // Set the threads as unstopped once they actually have stopped so that we can search again.
    for (int i = 0; i < threadsToSpawn; i++) workerDatas[i + 1].isSearchStopped = false;
    if (firstWorkerData.isSearchStopped) {
      isCurrentlySearching = true;
      break;
    }
    principalVariation.clear();
    retrievePrincipalVariation(inputBoard, principalVariation);
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    int timeElapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    std::cout << "info depth " << currentDepth << " score ";
    std::string displayedScore = std::to_string(score);
    if (20000 - std::abs(score) <= 63) {
      int mateIn = ceil((20000.0 - (double)std::abs(score)) / 2.0);
      mateIn *= (score < 0 ? -1 : 1);
      std::cout << "mate " << mateIn << " ";
    } else {
      std::cout << "cp " << score << " "; 
    }
    int nodes = 0;
    for (int i = 0; i < numberOfThreads; i++) nodes += workerDatas[i].nodes;
    int nodesPerSecond = (int)((double)nodes / ((double)timeElapsed / 1000.0));
    std::cout << "time " << timeElapsed << " nodes " << nodes << " nps " << nodesPerSecond << " pv ";
    int principalVariationSizeMinusOne = principalVariation.size() - 1;
    for (int i = 0; i <= principalVariationSizeMinusOne; i++) {
      std::cout << moveToLongAlgebraic(principalVariation[i]);
      if (i != principalVariationSizeMinusOne) std::cout << " ";
    }
    std::cout << std::endl;
    if (currentDepth == depthLimit) {
      isCurrentlySearching = false;
      break;
    }
    currentDepth++;
  }
  std::cout << "bestmove " << moveToLongAlgebraic(principalVariation[0]) << std::endl;
  isCurrentlySearching = false;
}
void prepareForSearch(board& inputBoard) {
  inputBoard.isSearchStopped = false;
  inputBoard.nodes = 0;
  inputBoard.distanceToRoot = 0;
  startTime = std::chrono::steady_clock::now();
  isInterruptedByGui = false;
  isCurrentlySearching = true;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      for (int k = 0; k < 64; k++) {
        inputBoard.historyTable[i][j][k] = 0;
      }
    }
  }
  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 3; j++) {
      inputBoard.killersTable[i][j] = 0;
    }
  }
}
 