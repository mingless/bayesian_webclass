angular.module('apkService', [])
	.service('srvCommands', //commands
	
	function($http) {
                 this.baseURL = client_server_prefix + '/ajax/calcpy/'; //the prefix is defined in version.js

                 this.getData = function(callback) {
		          return $http.get(this.baseURL + 'getData').success(callback);
                 };

		//this.startCommand = function(callback) {
		//	 return $http.get(this.baseURL + 'startCommand').success(callback);
		// };
         });

