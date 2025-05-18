

![image](https://github.com/user-attachments/assets/8a010cbe-a64a-4d51-abd5-fe58ee7e4ad5)

Fixed-priority multiprocessor scheduling with liu and layland's utilization bound 논문의 파티셔닝 알고리즘을 Pico위에서 구현및 실험

FREE RTOS SMP ver로 빌드해 RP2040의 멀티코어를 모두 활용하여 실험 진행

모든 TASK가 Periodic인 TASK SET에 대하여 해당 SET의 모든 TASK들의 주기(Tick 기준)의 최소 공배수 동안 SPA2 알고리즘을 통한 스케줄링을 진행하여 Overflow의 발생 여부를 확인















