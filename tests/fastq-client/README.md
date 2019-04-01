# A FASTQ producing tool

## Purpose:
This tool is suitable for spawning and driving the VDB-3 data process. Its purpose
is to drive the VDB-3 data process for the sake of developing and testing:
* Messages.
* Protocols.
* IPC.

## Description:
This is a basic tool that will try to open a cursor on the `SEQUENCE` table and print out:
1. `NAME`
2. `READ`
3. `QUALITY`
in FASTQ format.
