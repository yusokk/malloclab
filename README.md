C 언어를 이용한 Malloc Lab

- 참여자 : 김용기, 김유석, 이승민
- 프로젝트 기간 : 2021.01.15 ~ 2021.01.21
- 명시적 메모리 할당 :
  - C 표준 라이브러리는 malloc 패키지라고 하는 명시적 할당기 제공
  - Malloc 함수를 호출해 블록을 할당하며, free 함수를 호출해 블록을 반환
  - 동적 메모리 할당을 사용하는 이유 : 종종 프로그램을 실제 실행시키기 전에는 자료 구조의 크기를 알 수 없는 경우들이 있기 때문
- Implicit 방식 동적 메모리 할당 : 
  - 가용 블록이 헤더 내 필드에 의해 묵시적으로 연결
  - 단순성이 장점
  - 블록을 배치하는 것과 같은 가용 리스트를 탐색해야 하는 연산들의 비용이 힙에 있는 전체 할당된 블록과 가용 블록의 수에 비례 (범용성이 떨어짐)
  - 블록 구성 : 
    - 1워드 헤더(블록 크기, 가용 상태)
    - 데이터
    - 추가적인 패딩(외부 단편화 극복 전략, 정렬 요구사항 만족)
  - 더블 워드 정렬
  - First fit, next fit, best fit 중 first fit 적용
  - 경계 태그를 통한 상수 시간 내 블록 연결

* Explicit :
  * 가용 블록의 본체는 프로그램에서 필요하지 않기 때문에 이 자료구조를 구현하는 포인터들은 가용 블록의 본체 내에 저장 가능
  * Predecessor와 successor 포인터를 포함하는 이중 연결 가용 리스트 구성
  * LIFO tnstjdml first fit이 아닌 리스트를 주소 순으로 관리하는 형태로 구현

