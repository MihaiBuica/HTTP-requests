#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.c"

char* get_ip(char* name){
    int rez;
    struct addrinfo hints, *result, *p;
    char aux[BUFLEN];
    memset (&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    rez = getaddrinfo(name, NULL, &hints, &result);

    if(rez != 0){
        printf("getaddrinfo error!\n");
        return NULL;
    }
    for(p = result; p != NULL; p = p->ai_next){
        if(p->ai_family == AF_INET){
            struct sockaddr_in* addr = (struct sockaddr_in *) p->ai_addr;
            inet_ntop(p->ai_family, &addr->sin_addr, aux, sizeof(aux));
        }
    }

    char *ret = malloc(sizeof(char)*BUFLEN);
    if(!ret){
        printf("Malloc error!\n");
        return NULL;
    }
    strcpy(ret, aux);
    freeaddrinfo(result);
    return ret;
}

char** extract_cookie (char *message, int *nr_cookie){
    int dim = strlen("Set-Cookie: "); //e 12
    int length;
    int fin;
    char *start_cookie = strstr(message,"Set-Cookie: ");
    char** vector_cookie = (char**) calloc(sizeof(char*), 512);
    if(!vector_cookie){
        printf("Eroare alocare memorie!\n");
        return NULL;
    }
    

    int poz = dim;
    while(start_cookie != NULL){
        poz = dim + 1;
        while(start_cookie[poz] != ';'){ //pana unde se preia cookie
            poz++;
        }
        
        fin = poz - 1;
        length = fin - dim + 1;
        
        vector_cookie[(*nr_cookie)] = calloc(sizeof(char), length + 1);
        if(!vector_cookie[(*nr_cookie)]){
            printf("Eroare alocare memorie.\n");
            return NULL;
        }

        strncpy(vector_cookie[(*nr_cookie)], start_cookie + dim, length);
        vector_cookie[length + 1] = '\0';
        (*nr_cookie)++;

        start_cookie = strstr(start_cookie + 1, "Set-Cookie: ");
    }
    return vector_cookie;

}

char* get_json(JSON_Array *commits, char* str){
    JSON_Object *commit;
    size_t i;
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        return ( (char *) json_object_get_string(commit, str) );
    }
    return NULL;
}

char* get_json_data_user_pass(JSON_Array *commits){
    char *ret_str = calloc(sizeof(char), 512);
    ret_str[0] = '\0';
    JSON_Object *commit;
    size_t i;
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        JSON_Object *comm = json_object_get_object(commit,"data");
        int size = json_object_get_count(comm);
        for(int j = 0; j < size; j++){
            strcat(ret_str, json_object_get_name(comm,j));
            strcat(ret_str, "=");
            strcat(ret_str, json_object_get_string(comm, json_object_get_name(comm,j)));
            if(j != size - 1){                
                strcat(ret_str, "&");
            }  
        }
    }
    return ret_str;
}
char* get_json_data_token(JSON_Array *commits){
    JSON_Object *commit;
    size_t i;
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        JSON_Object *comm = json_object_get_object(commit,"data");
        int size = json_object_get_count(comm);
        for(int j = 0; j < size; j++){
            return((char *) json_object_get_string(comm, "token")); 
        }
    }
    return NULL;    
}
char* get_json_data_queryParamas_id(JSON_Array *commits){
    JSON_Object *commit;
    size_t i;
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        JSON_Object *comm = json_object_get_object(commit,"data");
        int size = json_object_get_count(comm);
        for(int j = 0; j < size; j++){
            JSON_Object *comm2 = json_object_get_object(comm,"queryParams");
            int size2 = json_object_get_count(comm2);
            for(int k = 0; k < size2; k++){
                return ((char *) json_object_get_string(comm2, json_object_get_name(comm2, k)));
            }

        }
    }
    return NULL;  
}
char* get_json_data_queryParamas(JSON_Array *commits){
    JSON_Object *commit;
    size_t i;
    char *response = calloc(sizeof(char), BUFLEN);
    response[0] = '\0';
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        JSON_Object *comm = json_object_get_object(commit,"data");
        int size = json_object_get_count(comm);
        for(int j = 0; j < size; j++){
            JSON_Object *comm2 = json_object_get_object(comm,"queryParams");
            int size2 = json_object_get_count(comm2);
            for(int k = 0; k < size2; k++){
                strcat(response, json_object_get_name(comm2,k));
                strcat(response, "=");
                strcat(response, json_object_get_string(comm2, json_object_get_name(comm2, k)));
                if(k != size2 - 1){
                    strcat(response, "&");
                }
            }
            return response;
        }
    }
    return NULL;  
}
char* get_json_data(JSON_Array *commits, char* str){
    JSON_Object *commit;
    size_t i;
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        JSON_Object *comm = json_object_get_object(commit,"data");
        int size = json_object_get_count(comm);
        for(int j = 0; j < size; j++){
            return((char *) json_object_get_string(comm, str)); 
        }
    }
    return NULL;    
}


void init_json_struct (char *json, JSON_Value **root_value, JSON_Array **commits){
    /* parsing json and validating output */
    *root_value = json_parse_string(json);
    if (json_value_get_type(*root_value) != JSONArray) {
        printf("Eroare get type\n");
        return;
    }

    /* getting array from root value and printing commit info */
    *commits = json_value_get_array(*root_value);
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    JSON_Value *root_value;
    JSON_Array *commits;
//================== ETAPA 1 ==================
    sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(IP_SERVER, "/task1/start", NULL, NULL, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    close_connection(sockfd);
    printf("\n================================FIRST=============================================\n");
    puts(response);
    
    int nr_cookie = 0;
    char *cookiev = malloc(sizeof(char) * BUFLEN);
    cookiev[0] = '\0';
    char **vector_cookie = extract_cookie(response, &nr_cookie);
    for(int i = 0; i < nr_cookie; i++){
        if(i != nr_cookie -1 ){
            strcat(cookiev, vector_cookie[i]);
            strcat(cookiev, "; ");
            free(vector_cookie[i]);
        }
        else{
            strcat(cookiev, vector_cookie[i]);
            free(vector_cookie[i]);
        }
    }


    // Prelucrare JSON

    char *json = strstr(response, "\n\r\n") + 3; //3 = dimensiunea "\n\r\n"
    char *json_vector = malloc(sizeof(char) * (strlen(json) + 3) );
    sprintf(json_vector, "[%s]", json); //cream vector de json pentru a putea fi parsat


    init_json_struct(json_vector, &root_value, &commits);
    if(strncmp(get_json(commits, "method"), "POST", 4) == 0){
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_post_request(IP_SERVER, get_json(commits, "url"), 
            get_json_data_user_pass(commits), get_json(commits, "type"), NULL, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    else{
         sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_get_request(IP_SERVER, "/task1/start", NULL, NULL, NULL);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }

    //================== ETAPA 2 ==================
    printf("\n================================SECOND=============================================\n");
    puts(response);
    free(cookiev);
    nr_cookie = 0;
    cookiev =  calloc(sizeof(char), BUFLEN);
    cookiev[0] = '\0';
    vector_cookie = extract_cookie(response, &nr_cookie);
    for(int i = 0; i < nr_cookie; i++){
        if(i != nr_cookie -1 ){
            strcat(cookiev, vector_cookie[i]);
            strcat(cookiev, "; ");
            free(vector_cookie[i]);

        }
        else{
            strcat(cookiev, vector_cookie[i]);
            free(vector_cookie[i]);
        }
    }

    free(json_vector);
    json = strstr(response, "\n\r\n") + 3; //3 = dimensiunea "\n\r\n"
    json_vector = malloc(sizeof(char) * (strlen(json) + 3) );
    sprintf(json_vector, "[%s]", json); //cream vector de json pentru a putea fi parsat

    init_json_struct(json_vector, &root_value, &commits);

    char *token = get_json_data_token(commits); //salvare token autentificare
    char *url = get_json(commits, "url");
    char *url_param = malloc(sizeof(char)*BUFLEN);
    url_param[0] = '\0';
    strcat(url_param, "raspuns1=");
    strcat(url_param, "omul");
    strcat(url_param, "&raspuns2=");
    strcat(url_param, "numele");
    strcat(url_param, "&id=");
    strcat(url_param, get_json_data_queryParamas_id(commits));
    //printf("%s\n", url_param);
    if(strncmp(get_json(commits, "method"), "POST", 4) == 0){
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_post_request(IP_SERVER, get_json(commits, "url"), 
            get_json_data_user_pass(commits), get_json(commits, "type"), token, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    else{
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_get_request(IP_SERVER, url, url_param, token, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }

    //================== ETAPA 3 ==================
    printf("\n================================THIRD=============================================\n");
    puts(response);
    free(cookiev);
    nr_cookie = 0;
    cookiev =  calloc(sizeof(char), BUFLEN);
    cookiev[0] = '\0';
    vector_cookie = extract_cookie(response, &nr_cookie);
    for(int i = 0; i < nr_cookie; i++){
        if(i != nr_cookie -1 ){
            strcat(cookiev, vector_cookie[i]);
            strcat(cookiev, "; ");
            free(vector_cookie[i]);

        }
        else{
            strcat(cookiev, vector_cookie[i]);
            free(vector_cookie[i]);
        }
    }
    //printf("Cookie: %s\n", cookiev);

    json = strstr(response, "\n\r\n") + 3; //3 = dimensiunea "\n\r\n"
    json_vector = malloc(sizeof(char) * (strlen(json) + 3) );
    sprintf(json_vector, "[%s]", json); //cream vector de json pentru a putea fi parsat

    init_json_struct(json_vector, &root_value, &commits);

    if(strncmp(get_json(commits, "method"), "POST", 4) == 0){
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_post_request(IP_SERVER, get_json(commits, "url"), 
            get_json_data_user_pass(commits), get_json(commits, "type"), token, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    else{
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_get_request(IP_SERVER, get_json(commits, "url"), NULL, token, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    //================== ETAPA 4 ==================
    printf("\n================================FOURTH=============================================\n");
    puts(response);
    free(cookiev);
    nr_cookie = 0;
    cookiev =  calloc(sizeof(char), BUFLEN);
    cookiev[0] = '\0';
    vector_cookie = extract_cookie(response, &nr_cookie);
    for(int i = 0; i < nr_cookie; i++){
        if(i != nr_cookie -1 ){
            strcat(cookiev, vector_cookie[i]);
            strcat(cookiev, "; ");
            free(vector_cookie[i]);

        }
        else{
            strcat(cookiev, vector_cookie[i]);
            free(vector_cookie[i]);
        }
    }

    json = strstr(response, "\n\r\n") + 3; //3 = dimensiunea "\n\r\n"
    json_vector = malloc(sizeof(char) * (strlen(json) + 3) );
    sprintf(json_vector, "[%s]", json); //cream vector de json pentru a putea fi parsat

    init_json_struct(json_vector, &root_value, &commits);


    char *string_url = get_json_data(commits, "url");

    char *file = strstr(string_url, "/");
    int len = file - string_url;
    char *adr = malloc(sizeof(char) * (len + 1));
    if (!adr){
        printf("Malloc error!\n");
        return -1;
    }
    strncpy(adr, string_url, len);
    adr[len+1] = '\0';
    char *ip_parm = get_ip(adr);
    char *respons_weather;
    char *json_weather;
    if(strncmp(get_json_data(commits, "method"), "POST", 4) == 0){
        sockfd = open_connection(ip_parm, PORT_HTTP, AF_INET, SOCK_STREAM, 0);
        message = compute_post_request(ip_parm, file, 
            get_json_data_queryParamas(commits), NULL, NULL, NULL);
        send_to_server(sockfd, message);
        respons_weather = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    else{
        sockfd = open_connection(ip_parm, PORT_HTTP, AF_INET, SOCK_STREAM, 0);
        message = compute_get_request(ip_parm, file, 
            get_json_data_queryParamas(commits), NULL, NULL);
        send_to_server(sockfd, message);
        respons_weather = receive_from_server(sockfd);
        close_connection(sockfd);
    }

    json_weather = strstr(respons_weather, "\n\r\n") + 3;
    if(strncmp(get_json(commits, "method"), "POST", 4) == 0){
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_post_request(IP_SERVER, get_json(commits, "url"), 
           json_weather, get_json(commits, "type"), token, cookiev);
        printf("Msaj: %s\n", message);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    else{
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        message = compute_get_request(IP_SERVER, get_json(commits, "url"), NULL, token, cookiev);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        close_connection(sockfd);
    }
    printf("\n================================FIFTH=============================================\n");   
    puts(response);

    free(message);
    return 0;
}
