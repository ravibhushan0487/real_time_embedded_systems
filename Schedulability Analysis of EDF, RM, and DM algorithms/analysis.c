//################################################################################################
//
// Program     : Scheduling Assignment 1
// Source file : analysis.c
// Authors     : Yash Shah and Ravi Bhushan Team #3
// Date        : 2nd March 2018
//
//################################################################################################


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define WRONG_INPUT_CHOICE -1
#define MAX_CPU_UTILIZATION 1.0f
#define SORT_BY_PERIOD 1
#define SORT_BY_DEADLINE 2
#define MAX( a, b ) ( ( a > b) ? a : b )
#define MIN( a, b ) ( ( a < b) ? a : b )

//Comparative Analysis Constants
#define DEADLINE_DISTRIBUTION_1 3 //For deadline distribution between [Ci,Ti]
#define DEADLINE_DISTRIBUTION_2 4 //For deadline distribution between [Ci+(Ti-Ci)/2,Ti]
#define TASK_IN_SET_1 10
#define TASK_IN_SET_2 25
#define MAX_SAMPLES 5000
#define M 3

//Comparative Analysis Variables
int edf_passed;
int rm_passed;
int dm_passed;
float utilization_set_1[TASK_IN_SET_1];
float utilization_set_2[TASK_IN_SET_2];
float DEADLINE_UPPER_BOUND;
float DEADLINE_LOWER_BOUND;

typedef struct file_data file_data;

struct file_data{
    char *data;
    file_data *next;
};
file_data *input;

typedef struct task_set task_set;
struct task_set{
	float worst_case_execution_time;
	float deadline;
	float period;
	task_set *next;
};

typedef struct lf_deadlines lf_deadlines;
struct lf_deadlines{
	lf_deadlines *previous;
	float deadline;
	lf_deadlines *next;
};

//Variable to denote if report needs to be printed in console
bool report_in_console = true;

int userOption;
int number_of_task_set;
//Is Task Set schedulable under Response Time Analysis
bool rt_analysis_schedulable = true;

/* function prototypes */
task_set* SortedMerge(task_set* a, task_set* b, int sort_by);
void FrontBackSplit(task_set* source, task_set** frontRef, task_set** backRef);
 
/* sorts the task set */
void MergeSort(task_set** headRef, int sort_by);

//Reads data from input file
file_data* fileReader(FILE *file){
    char fileData[20];
    int fileRead = fscanf(file,"%s",fileData);

    if(fileRead == EOF) {
        return NULL;
    }

    file_data *read_data = (file_data*)malloc(sizeof(file_data));
    read_data->data = strdup(fileData);
    read_data->next = fileReader(file);
    return read_data;
}

file_data* get_task_data() {
	FILE *inputFile = fopen("inputfile.txt","r");
     if(inputFile == NULL)   
    {      
       printf("\nFile opening failed ");      
       return NULL ;   
    }
    file_data *head = fileReader(inputFile);
    fclose(inputFile);
    return head;
}

/*void printLinkedList(file_data *threadData){
    printf("%s\n",threadData->data);
    if(threadData->next != NULL){
        printLinkedList(threadData->next);
    }
}*/

void printTaskSet(task_set *task){
    printf("%.2f ",task->worst_case_execution_time);
    printf("%.2f ",task->deadline);
    printf("%.2f\n",task->period);
    if(task->next != NULL){
        printTaskSet(task->next);
    }
}

task_set* get_task_set(int task_remaining) {
	if(task_remaining == 0){
		return NULL;
	}else {
		task_set* task_data = (task_set*)malloc(sizeof(task_set));
		task_data->worst_case_execution_time = atof(input->data);
		input = input->next;
		task_data->deadline = atof(input->data);
		input = input->next;
		task_data->period = atof(input->data);
		input = input->next;
		task_data->next = get_task_set(task_remaining -1);
		return task_data;
	}
} 

bool check_tasks_deadlines(task_set *task) {
	if(task == NULL){
		//None of the tasks have deadline greater than equal to period
		return true;
	}else if(task->deadline == task->period) {
		//One of the tasks has deadline equal to period
		return false;
	}else{
		bool deadline_less_than_period = check_tasks_deadlines(task->next);
		return deadline_less_than_period;
	}
}

float get_density(task_set *task) {
	if(task == NULL) {
		return 0.0f;
	} else {
		return task->worst_case_execution_time/MIN(task->period,task->deadline) + get_density(task->next);
	}
}

float get_busy_period(task_set *task, float busy_period) {
	if(task == NULL) {
		return 0.0f;
	}else {
		if(busy_period == 0.0f) {
			return task->worst_case_execution_time + get_busy_period(task->next,busy_period);
		} else {
			return ceil(busy_period/task->period)*task->worst_case_execution_time + get_busy_period(task->next,busy_period);
		}
		
	}
}

task_set* get_task(task_set *tasks, int task_number){
	if(task_number == 1){
		task_set *rt_task = (task_set*)malloc(sizeof(task_set));
		rt_task->worst_case_execution_time = tasks->worst_case_execution_time;
		rt_task->deadline = tasks->deadline;
		rt_task->period = tasks->period;
		return rt_task;
	}else {
		return get_task(tasks->next,task_number - 1);
	}
}

lf_deadlines* add_new_deadline(lf_deadlines *new_lf_deadline, lf_deadlines *lf_deadlines_list) {
	lf_deadlines *lf_deadlines_head = lf_deadlines_list;
	while(lf_deadlines_list != NULL) {
		if(new_lf_deadline->deadline == lf_deadlines_list->deadline) {
			break;
		} else if(new_lf_deadline->deadline > lf_deadlines_list->deadline) {
			if(lf_deadlines_list->next == NULL) {
				lf_deadlines_list->next = new_lf_deadline;
				new_lf_deadline->previous = lf_deadlines_list;
				break;
			}else {
				lf_deadlines_list = lf_deadlines_list->next;
			}
		} else if(new_lf_deadline->deadline < lf_deadlines_list->deadline) {
			if(lf_deadlines_list->previous == NULL) {
				new_lf_deadline->next = lf_deadlines_list;
				lf_deadlines_list->previous = new_lf_deadline;
				lf_deadlines_head = new_lf_deadline;
				break;
			}else {
				new_lf_deadline->next = lf_deadlines_list;
				new_lf_deadline->previous = lf_deadlines_list->previous;
				lf_deadlines_list->previous = new_lf_deadline;
				lf_deadlines_list = lf_deadlines_list->previous;
				lf_deadlines_list = lf_deadlines_list->previous;
				lf_deadlines_list->next = new_lf_deadline;
				break;
			}
		}
	}
	return lf_deadlines_head;
}

lf_deadlines* get_deadline_instances(task_set* tasks,lf_deadlines *lf_deadlines_list, float busy_period) {
	if(tasks == NULL){
		return lf_deadlines_list;
	} else {
		float task_deadline = tasks->deadline;
		float task_period = tasks->period;
		if(lf_deadlines_list->deadline == 0.0f) {
			lf_deadlines_list->deadline = task_deadline;
			lf_deadlines_list->previous = NULL;
			lf_deadlines_list->next = NULL;
			task_deadline = task_deadline + task_period;
		}
		while(task_deadline <= busy_period) {
			lf_deadlines *new_lf_deadline = (lf_deadlines*)malloc(sizeof(lf_deadlines));
			new_lf_deadline->previous = NULL;
			new_lf_deadline->next = NULL;
			new_lf_deadline->deadline = task_deadline;
			lf_deadlines_list = add_new_deadline(new_lf_deadline,lf_deadlines_list);
			task_deadline = task_deadline + task_period;
		}
		tasks = tasks->next;
		return get_deadline_instances(tasks,lf_deadlines_list,busy_period);
	}
}

void print_deadline_instances(lf_deadlines *lf_deadlines_list){
    printf("%.0f ",lf_deadlines_list->deadline);
    if(lf_deadlines_list->next != NULL){
        print_deadline_instances(lf_deadlines_list->next);
    }else {
    	printf("\n");
    }
}

void check_loading_factor(task_set *tasks, lf_deadlines *lf_deadlines_list) {
	bool deadline_was_missed = false;
	float missed_deadline = 0.0f;
	while(lf_deadlines_list != NULL) {
		float processor_demand = 0.0f;
		float loading_factor = 0.0f;
		float lf_deadline = lf_deadlines_list->deadline;
		task_set *task_details = tasks;
		while(task_details != NULL) {
			float period = task_details->period;
			float task_deadline = floor(lf_deadline/period)*period + task_details->deadline;
			if(task_deadline <= lf_deadline) {
				processor_demand = processor_demand + ceil(lf_deadline/period)*task_details->worst_case_execution_time;
			} else {
				processor_demand = processor_demand + floor(lf_deadline/period)*task_details->worst_case_execution_time;
			}
			task_details = task_details->next;
		}
		loading_factor = processor_demand/lf_deadlines_list->deadline;
		if(report_in_console) {
			printf("Deadline %.2f has Loading Factor of %0.4f\n",lf_deadline,loading_factor);
		}
		if(loading_factor > 1) {
			deadline_was_missed = true;
			missed_deadline = lf_deadlines_list->deadline;
			break;
		}
		lf_deadlines_list = lf_deadlines_list->next;
	}

	if(deadline_was_missed) {
		if(report_in_console) {
			printf("Deadline was missed at %0.2f\n. Task set is not schedulable under EDF\n",missed_deadline);
		}
	} else {
		if(report_in_console) {
			printf("All deadlines were met. Task set is Schedulable under EDF.\n");
		}
		edf_passed++;
	}
}

void apply_Loading_Factor_analysis(task_set *tasks, int number_of_tasks){
	if(report_in_console) {
		printf("Loading Factor Analysis\n");
		printf("***********************\n");
	}
	float busy_period = 0.0f;
	float current_busy_period = 0.0f;
	task_set *task_set_head = tasks;
	task_set *task_set_head2 = tasks;
	while(true) {
		if(busy_period == 0.0f) {
			current_busy_period = tasks->worst_case_execution_time + get_busy_period(tasks->next,busy_period);
			busy_period = current_busy_period;
		}else {
			current_busy_period = ceil(busy_period/tasks->period)*tasks->worst_case_execution_time + get_busy_period(tasks->next,busy_period);
			if(current_busy_period == busy_period) {
				break;
			}else{
				busy_period = current_busy_period;
			}
		}
		//printf("busy period %.0f\n",busy_period);
	}
	if(report_in_console) {
		printf("Busy Period of this task set is %.0f\n",busy_period);
		printf("Testing Loading Factor at Deadline instances less than %.0f\n",busy_period);
	}
	if (isinf(busy_period) || isnan(busy_period)) {
		if(report_in_console) {
			printf("Busy Period calculation inconclusive. Unable to apply EDF.\n");
		}
	} else {
		lf_deadlines *lf_deadlines_list = (lf_deadlines*)malloc(sizeof(lf_deadlines));
		lf_deadlines_list->deadline = 0.0f;
		lf_deadlines_list = get_deadline_instances(task_set_head, lf_deadlines_list, busy_period);
		if(report_in_console) {
			printf("Deadline Instances:");
			print_deadline_instances(lf_deadlines_list);
		}
		check_loading_factor(task_set_head2,lf_deadlines_list);
	}
	
}

float get_cpu_utilization_factor(task_set *task,int number_of_tasks) {
	if(task == NULL || number_of_tasks == 0) {
		return 0.0f;
	}else {
		//printf("Task utilization factor ::%f\n",task->worst_case_execution_time/task->period);
		return task->worst_case_execution_time/task->period + get_cpu_utilization_factor(task->next,number_of_tasks - 1);
	}
}

void apply_EDF_analysis(task_set *tasks,int number_of_tasks) {
	if(report_in_console) {
		printf("\nAnaysing the task set schedulability based on EDF\n");
		printf("-------------------------------------------------\n\n");
	}
	bool deadline_less_than_period = check_tasks_deadlines(tasks);
	if(deadline_less_than_period) {
		//Check for Density
		float density = get_density(tasks);
		if(report_in_console) {
			printf("The Deadlines of all the tasks in this task set is less than their time periods\n");
			printf("The Density of tasks in this task set is %f\n",density);
		}	
		if(density <= MAX_CPU_UTILIZATION) {
			if(report_in_console) {
				printf("Since this is less than equal to Maximum CPU Utilization factor of %f, this task set is schedulable under EDF Algorithm\n\n\n",MAX_CPU_UTILIZATION);
			}
			edf_passed++;
		} else {
			if(report_in_console) {
				printf("Since this is more than Maximum CPU Utilization factor of %f, we need to do Loading Factor Analysis\n",MAX_CPU_UTILIZATION);
			}
			apply_Loading_Factor_analysis(tasks, number_of_tasks);
		}
	} else {
		//Check for CPU Utilization
		float utilization_factor = get_cpu_utilization_factor(tasks, number_of_tasks);
		if(report_in_console) {
			printf("The Deadlines of some of the tasks in this task set is equal to their time period\n");
			printf("The CPU Utilization of this task set is %f\n",utilization_factor);
		}
		if(utilization_factor <= MAX_CPU_UTILIZATION) {
			if(report_in_console) {
				printf("Since this is less than equal to Maximum CPU Utilization factor of %f, this task set is schedulable under EDF Algorithm\n\n\n",MAX_CPU_UTILIZATION);
			}
			edf_passed++;
		}else {
			if(report_in_console) {
				printf("Since this is more than Maximum CPU Utilization factor of %f, this task set is not schedulable under EDF Algorithm\n\n\n",MAX_CPU_UTILIZATION);
			}
		}
	}
}

float get_utilization_bound(float number_of_tasks) {
	//printf("Utilization Bound %f\n",number_of_tasks*(pow(2,(1/number_of_tasks))-1));
	return number_of_tasks*(pow(2,(1/number_of_tasks))-1);
}

float get_response_time(task_set *task, int number_of_tasks,float response_time) {
	if(task == NULL || number_of_tasks == 0) {
		return 0.0f;
	}else {
		//printf("Execution Time ::%f\n",task->worst_case_execution_time);
		if(response_time == 0.0f) {
			return task->worst_case_execution_time + get_response_time(task->next,number_of_tasks - 1,response_time);
		} else {
			return ceil(response_time/task->period)*task->worst_case_execution_time + get_response_time(task->next,number_of_tasks - 1,response_time);
		}
		
	}
}

void perform_RT(task_set *tasks,int task_number) {
	float response_time = 0.0f;
	task_set *rt_task = get_task(tasks,task_number);
	while(true) {
		float current_response_time = rt_task->worst_case_execution_time + get_response_time(tasks,task_number-1,response_time);
		if(current_response_time == response_time) {
			break;
		}else {
			response_time = current_response_time;
		}
	}
	if(response_time <= rt_task->period) {
		if(report_in_console) {
			printf("Task %d has Worse Case Rrsponse Time of %f which is less than equals to its period %f. So, it is schedulable under RT analysis\n",task_number,response_time,rt_task->period);
		}
	} else {	
		if(report_in_console) {
			printf("Task %d has Worse Case Response Time of %f which is greater than its period %f. So, it is not schedulable under RT analysis\n",task_number,response_time,rt_task->period);
		}
		rt_analysis_schedulable = false;
	}
}

void apply_Response_Time_analysis(task_set *tasks, int number_of_tasks, bool is_rm_analysis){
	if(report_in_console) {
		printf("Response Time Analysis\n");
		printf("**********************\n");
	}
	int task_number = 1;
	bool performed_RT_analysis = false;
	rt_analysis_schedulable = true;
	while(task_number <= number_of_tasks) {
		float utilization_bound = get_utilization_bound((float)task_number);
		float utilization_factor = get_cpu_utilization_factor(tasks,task_number);
		if(utilization_factor <= utilization_bound) {
			if(task_number == 1){
				if(report_in_console) {
					printf("The first task is having Utilization Factor of %f, which is less than Utilization Bound of %f. So, it schedulable.\n",utilization_factor,utilization_bound);
				}
			}else {
				if(report_in_console) {
					printf("The first %d tasks are having Utilization Factor of %f, which is less than Utilization Bound of %f. So, they are schedulable.\n",task_number,utilization_factor,utilization_bound);
				}
			}
		}else {
				performed_RT_analysis = true;
				if(report_in_console) {
					printf("The first %d tasks are having Utilization Factor of %f, which is greater than Utilization Bound of %f.\n",task_number,utilization_factor,utilization_bound);
					printf("Need to do Response Time Analysis on %drd task\n",task_number);
				}
				perform_RT(tasks,task_number);
				if(!rt_analysis_schedulable) {
					break;
				}
		}
		task_number++;
	}
	if(performed_RT_analysis && rt_analysis_schedulable) {
		if(report_in_console) {
			printf("Given Task Set is schedulable under Response Time Analysis. Hence, it is schedulable\n");
		}
		if(is_rm_analysis) {
			rm_passed++;
		}else {
			dm_passed++;
		}
	}else if(!performed_RT_analysis) {
		if(report_in_console) {
			printf("Given Task Set is schedulable under Response Time Analysis. Hence, it is schedulable\n");
		}
		if(is_rm_analysis) {
			rm_passed++;
		}else {
			dm_passed++;
		}
	}else {
		if(report_in_console) {
			printf("Given Task Set is not schedulable under Response Time Analysis. Hence, it is not schedulable\n");
		}
	}
}

//Check if Deadline and Period Orders are different in the sorted task list
bool check_for_order(task_set *tasks,int sort_by) {
	float previous_element = 0.0f;
	while(tasks != NULL) {
		if(sort_by == SORT_BY_DEADLINE) {
			if(previous_element > tasks->deadline) {
				//printf("Deadline Order different from Period Order\n");
				return true;//Order changed
			}else{
				previous_element = tasks->deadline;
			}
		}else if(sort_by == SORT_BY_PERIOD) {
			if(previous_element > tasks->period) {
				//printf("Deadline Order different from Period Order\n");
				return true;//Order changed
			}else {
				previous_element = tasks->period;
			}
		}
		tasks = tasks->next;
	}
	return false;
}

void apply_RM_analysis(task_set *tasks, int number_of_tasks) {
	if(report_in_console) {
		printf("\nAnaysing the task set schedulability based on RM\n");
		printf("-------------------------------------------------\n\n");
	}
	//printTaskSet(tasks);
	bool deadline_less_than_period = check_tasks_deadlines(tasks);
	if(deadline_less_than_period) {
		bool sorting_changed_priority = check_for_order(tasks,SORT_BY_DEADLINE);
		if(sorting_changed_priority) {
			apply_Response_Time_analysis(tasks,number_of_tasks,true);
		} else {
			//Check for CPU Utilization
			float utilization_factor = get_cpu_utilization_factor(tasks,number_of_tasks);
			float utilization_bound = get_utilization_bound((float)number_of_tasks);
			if(report_in_console) {
				printf("The CPU Utilization of this task set is %f\n",utilization_factor);
				printf("The Utilization Bound of this task set is %f\n",utilization_bound);
			}
			if(utilization_factor <= utilization_bound) {
				if(report_in_console) {
					printf("Since CPU Utilization is less than equal to Utilization Bound, this task set is schedulable under RM Algorithm\n\n\n");
				}
				rm_passed++;
			}else {
				if(report_in_console) {
					printf("Since CPU Utilization is more than Utilization Bound, we need to prform Response Time Test for ths task set\n\n");
				}
				apply_Response_Time_analysis(tasks,number_of_tasks,true);
			}
		}
	} else {
		//Check for CPU Utilization
		float utilization_factor = get_cpu_utilization_factor(tasks,number_of_tasks);
		float utilization_bound = get_utilization_bound((float)number_of_tasks);
		if(report_in_console) {
			printf("The Deadlines of some of the tasks in this task set is equal to their time period\n");
			printf("The CPU Utilization of this task set is %f\n",utilization_factor);
			printf("The Utilization Bound of this task set is %f\n",utilization_bound);
		}
		if(utilization_factor <= utilization_bound) {
			if(report_in_console) {
				printf("Since CPU Utilization is less than equal to Utilization Bound, this task set is schedulable under RM Algorithm\n\n\n");
			}
		}else {
			if(report_in_console) {
				printf("Since CPU Utilization is more than Utilization Bound, we need to prform Response Time Test for ths task set\n\n");
			}
			apply_Response_Time_analysis(tasks,number_of_tasks,true);
		}
	}
}

void apply_DM_analysis(task_set *tasks, int number_of_tasks) {
	if(report_in_console) {
		printf("\nAnaysing the task set schedulability based on DM\n");
		printf("-------------------------------------------------\n\n");
	}
	//printTaskSet(tasks);
	//rt_analysis_schedulable = true;
	bool deadline_less_than_period = check_tasks_deadlines(tasks);
	if(deadline_less_than_period) {
		bool sorting_changed_priority = check_for_order(tasks,SORT_BY_PERIOD);
		if(sorting_changed_priority) {
			apply_Response_Time_analysis(tasks,number_of_tasks,false);
		} else {
			//Check for CPU Utilization
			float utilization_factor = get_cpu_utilization_factor(tasks,number_of_tasks);
			float utilization_bound = get_utilization_bound((float)number_of_tasks);
			if(report_in_console) {
				printf("The Deadlines of some of the tasks in this task set is equal to their time period\n");
				printf("The CPU Utilization of this task set is %f\n",utilization_factor);
				printf("The Utilization Bound of this task set is %f\n",utilization_bound);
			}
			if(utilization_factor <= utilization_bound) {
				if(report_in_console) {
					printf("Since CPU Utilization is less than equal to Utilization Bound, this task set is schedulable under DM Algorithm\n\n\n");
				}
			}else {
				if(report_in_console) {
					printf("Since CPU Utilization is more than Utilization Bound, we need to perform Response Time Test for ths task set\n\n");
				}
				apply_Response_Time_analysis(tasks,number_of_tasks,false);
			}
		}
	} else {
		//Check for CPU Utilization
		float utilization_factor = get_cpu_utilization_factor(tasks,number_of_tasks);
		float utilization_bound = get_utilization_bound((float)number_of_tasks);
		if(report_in_console) {
			printf("The Deadlines of some of the tasks in this task set is equal to their time period\n");
			printf("The CPU Utilization of this task set is %f\n",utilization_factor);
			printf("The Utilization Bound of this task set is %f\n",utilization_bound);
		}
		if(utilization_factor <= utilization_bound) {
			if(report_in_console) {
				printf("Since CPU Utilization is less than equal to Utilization Bound, this task set is schedulable under DM Algorithm\n\n\n");
			}
		}else {
			if(report_in_console) {
				printf("Since CPU Utilization is more than Utilization Bound, we need to perform Response Time Test for ths task set\n\n");
			}
			apply_Response_Time_analysis(tasks,number_of_tasks,false);
		}
	}
}

void UUniFast(int total_tasks,float u_bar){
	int i;

	float sumU=u_bar;
	float nextSumU;

				
		for(i=0;i<total_tasks-1;i++)
		{
			nextSumU=sumU*pow((rand()/(RAND_MAX*1.0)),(1.0/(total_tasks-i)));
			if(total_tasks == TASK_IN_SET_1) {
				utilization_set_1[i]=sumU-nextSumU;
			}else if(total_tasks == TASK_IN_SET_2){
				utilization_set_2[i]=sumU-nextSumU;
			}
			sumU=nextSumU;
		}	
		if(total_tasks == TASK_IN_SET_1) {
			utilization_set_1[total_tasks-1]=sumU;
		}else if(total_tasks == TASK_IN_SET_2){
			utilization_set_2[total_tasks-1]=sumU;
		}
}

//Plot task set
void plot_graph(float u_bar,int edf_passed, int rm_passed, int dm_passed) {
	printf("Utilization :%.2f  EDF Passed :%d  RM Passed : %d DM Passed : %d\n",u_bar,edf_passed,rm_passed,dm_passed);
}

//Generate Synthetic task set and analysing them
void task_analysis (int number_of_tasks,int deadline_distribution) {
	float U_BAR=0.05;
	printf("Analysis of Synthetic Task Sets\n");
	while(U_BAR<=1) {
		edf_passed = 0;
		rm_passed = 0;
		dm_passed = 0;
		int SAMPLE=0;
		//Generate Task Set
		while(SAMPLE++<MAX_SAMPLES) {
			task_set *task_set_head = (task_set*)malloc(sizeof(task_set));
			task_set *previous = (task_set*)malloc(sizeof(task_set));
			task_set_head->next = NULL;
			for(int i=0;i<number_of_tasks;i++)
			{
				if(i != 0) {
					task_set *tasks = (task_set*)malloc(sizeof(task_set));
					
					//Generating Period
					if(i<=number_of_tasks/3.0)
						tasks->period=rand()/(RAND_MAX*1.0)*(100-10)+10;
					else if(i>number_of_tasks/3.0 && i<(2.0/3.0)*number_of_tasks)
						tasks->period=rand()/(RAND_MAX*1.0)*(1000-100)+100;
					else
						tasks->period=rand()/(RAND_MAX*1.0)*(10000-1000)+1000;

					//Generating Utilization,Execution time and Deadline
					if(number_of_tasks == TASK_IN_SET_1) {
						UUniFast(number_of_tasks,U_BAR);
						tasks->worst_case_execution_time=utilization_set_1[i]*tasks->period;
				
						DEADLINE_UPPER_BOUND=tasks->period*1.0;
						DEADLINE_LOWER_BOUND=tasks->worst_case_execution_time+((deadline_distribution - DEADLINE_DISTRIBUTION_1)*((tasks->period - tasks->worst_case_execution_time)/2.0));
						tasks->deadline=rand()/(RAND_MAX*1.0)*(DEADLINE_UPPER_BOUND-DEADLINE_LOWER_BOUND)+DEADLINE_LOWER_BOUND;
				
					}else if(number_of_tasks == TASK_IN_SET_2){
						UUniFast(number_of_tasks,U_BAR);
						tasks->worst_case_execution_time=utilization_set_2[i]*tasks->period;
				
						DEADLINE_UPPER_BOUND=tasks->period*1.0;
						DEADLINE_LOWER_BOUND=tasks->worst_case_execution_time+((deadline_distribution - DEADLINE_DISTRIBUTION_1)*((tasks->period - tasks->worst_case_execution_time)/2.0));
						tasks->deadline=rand()/(RAND_MAX*1.0)*(DEADLINE_UPPER_BOUND-DEADLINE_LOWER_BOUND)+DEADLINE_LOWER_BOUND;
				
					}
					previous->next = tasks;
					previous = previous->next;
					
				} else {
					//Generating Period
					if(i<=number_of_tasks/3.0)
						task_set_head->period=rand()/(RAND_MAX*1.0)*(100-10)+10;
					else if(i>number_of_tasks/3.0 && i<(2.0/3.0)*number_of_tasks)
						task_set_head->period=rand()/(RAND_MAX*1.0)*(1000-100)+100;
					else
						task_set_head->period=rand()/(RAND_MAX*1.0)*(10000-1000)+1000;

					//Generating Utilization,Execution time and Deadline
					if(number_of_tasks == TASK_IN_SET_1) {
						UUniFast(number_of_tasks,U_BAR);
						task_set_head->worst_case_execution_time=utilization_set_1[i]*task_set_head->period;
				
						DEADLINE_UPPER_BOUND=task_set_head->period*1.0;
						DEADLINE_LOWER_BOUND=task_set_head->worst_case_execution_time+((deadline_distribution - DEADLINE_DISTRIBUTION_1)*((task_set_head->period - task_set_head->worst_case_execution_time)/2.0));
						task_set_head->deadline=rand()/(RAND_MAX*1.0)*(DEADLINE_UPPER_BOUND-DEADLINE_LOWER_BOUND)+DEADLINE_LOWER_BOUND;
				
					}else if(number_of_tasks == TASK_IN_SET_2){
						UUniFast(number_of_tasks,U_BAR);
						task_set_head->worst_case_execution_time=utilization_set_2[i]*task_set_head->period;
				
						DEADLINE_UPPER_BOUND=task_set_head->period*1.0;
						DEADLINE_LOWER_BOUND=task_set_head->worst_case_execution_time+((deadline_distribution - DEADLINE_DISTRIBUTION_1)*((task_set_head->period - task_set_head->worst_case_execution_time)/2.0));
						task_set_head->deadline=rand()/(RAND_MAX*1.0)*(DEADLINE_UPPER_BOUND-DEADLINE_LOWER_BOUND)+DEADLINE_LOWER_BOUND;
				
					}
					previous = task_set_head;
				}
			}
			//Sort Task Set based on deadline
			MergeSort(&task_set_head,SORT_BY_DEADLINE);
			apply_EDF_analysis(task_set_head,number_of_tasks);

			//Sort Task Set based on deadline
			MergeSort(&task_set_head,SORT_BY_PERIOD);
			apply_RM_analysis(task_set_head,number_of_tasks);
			
			//Sort Task Set based on deadline
			MergeSort(&task_set_head,SORT_BY_DEADLINE);
			apply_DM_analysis(task_set_head,number_of_tasks);
		}
		plot_graph(U_BAR,edf_passed,rm_passed,dm_passed);	
		U_BAR+=0.1;
	}
}

//Automated Comparative Analysis
void comparative_analysis() {
	printf("This section is for comparative analysis\n");
	report_in_console = false;
	task_analysis(TASK_IN_SET_1,DEADLINE_DISTRIBUTION_1);
	task_analysis(TASK_IN_SET_2,DEADLINE_DISTRIBUTION_1);
	task_analysis(TASK_IN_SET_1,DEADLINE_DISTRIBUTION_2);
	task_analysis(TASK_IN_SET_2,DEADLINE_DISTRIBUTION_2);
}

void test_task_schedulability() {
	report_in_console = true;
	printf("Trying to read data from inputfile.txt\n\n");
	input = get_task_data();
	if(input == NULL){
		return;
	}
	number_of_task_set = atoi(input->data);
	printf("Total Task Sets : %d\n\n",number_of_task_set);
	input = input->next;
	for(int counter = 0;counter<number_of_task_set;counter++){
		int number_of_tasks = atoi(input->data);
		printf("This is Task Set : %d\n",counter+1);
		printf("--------------------\n");
		input = input->next;
		task_set *tasks = get_task_set(number_of_tasks);
		printTaskSet(tasks);
		
		//Sort Task Set based on deadline
		MergeSort(&tasks,SORT_BY_DEADLINE);
		apply_EDF_analysis(tasks,number_of_tasks);

		//Sort Task Set based on deadline
		MergeSort(&tasks,SORT_BY_PERIOD);
		apply_RM_analysis(tasks,number_of_tasks);
		
		//Sort Task Set based on deadline
		MergeSort(&tasks,SORT_BY_DEADLINE);
		apply_DM_analysis(tasks,number_of_tasks);
	}
	return;
}

int show_menu(){

	int option,count=0;
	char buffer;
 	printf("\t\t\tMenu\n");
    printf("\t\t\t----\n");
    printf("\t1 - Schedulability Testing\n");
    printf("\t2 - Comparative Analysis of EDF, RM and DM Algorithms\n");
    printf("\t3 - Exit");
    printf("\n\nChoose Your Option :");
    while((buffer = getchar())!= '\n'){
    	option = atoi(&buffer);
    	count++;
    };
    if(count > 1){
    	return WRONG_INPUT_CHOICE;
    } else {
    	return option;
    }
    //int user_choice = atoi(&option);
    printf("you chose %d",option);
    return option;
}

int main(int argc, char **argv)
{
    printf("\t\t\t\t\tSchedulability Analysis\n");
    printf("\t\t\t\t\t-----------------------\n");
    
    userOption = show_menu();
    
    while(userOption != 3) {

    	switch(userOption) {

   			case 3 :
   				  break;
   			case 1 :
			      test_task_schedulability();
			      break;
	
		    case 2 :
		      	  comparative_analysis();
		      	  break;
  
    		default :
    			  printf("\n\nWrong Option Selected\n");
    			  break;
		}
    	userOption = show_menu();
    }
	
	return 0;
}


/* sorts the task set linked list */
void MergeSort(task_set** headRef, int sort_by) {
  task_set* head = *headRef;
  task_set* a;
  task_set* b;
 
  /* Base case -- length 0 or 1 */
  if ((head == NULL) || (head->next == NULL))
  {
    return;
  }
 
  /* Split head into 'a' and 'b' sublists */
  FrontBackSplit(head, &a, &b); 
 
  /* Recursively sort the sublists */
  MergeSort(&a, sort_by);
  MergeSort(&b, sort_by);
 
  /* Merges the two sorted lists together */
  *headRef = SortedMerge(a, b, sort_by);
}
 
/* Merges the two sorted lists together */
task_set* SortedMerge(task_set* a, task_set* b, int sort_by) {
  struct task_set* result = NULL;
  float sort_data_a;
  float sort_data_b;
  /* Base cases */
  if (a == NULL)
     return(b);
  else if (b==NULL)
     return(a);
 
  if(sort_by == SORT_BY_DEADLINE) {
  	sort_data_a = a->deadline;
  	sort_data_b = b->deadline;
  } else {
	sort_data_a = a->period;
  	sort_data_b = b->period;
  }
 /* Pick either a or b, and recur */
  if (sort_data_a <= sort_data_b)
  {
     result = a;
     result->next = SortedMerge(a->next, b, sort_by);
  }
  else
  {
     result = b;
     result->next = SortedMerge(a, b->next, sort_by);
  }
  return(result);
}
 
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves,
     and return the two lists using the reference parameters.
     If the length is odd, the extra node should go in the front list.
     Uses the fast/slow pointer strategy.  */
void FrontBackSplit(task_set* source, task_set** frontRef, task_set** backRef) {
  struct task_set* fast;
  struct task_set* slow;
  if (source==NULL || source->next==NULL)
  {
    /* length < 2 cases */
    *frontRef = source;
    *backRef = NULL;
  }
  else
  {
    slow = source;
    fast = source->next;
 
    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL)
    {
      fast = fast->next;
      if (fast != NULL)
      {
        slow = slow->next;
        fast = fast->next;
      }
    }
 
    /* 'slow' is before the midpoint in the list, so split it in two
      at that point. */
    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
  }
}