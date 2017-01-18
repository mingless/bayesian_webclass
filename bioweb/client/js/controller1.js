var apk = angular.module('apk',	[]);

var link = "";
apk.service('srvCommands', //commands

	function($http) {
                this.baseURL = client_server_prefix + '/ajax/calcpy/'; //the prefix is defined in version.js
                
                this.serverData = "";
                this.getDataServer = function(callback, value) {
                	
                	this.serverData =  $http.get(this.baseURL + 'greet?word=' + value).success(callback);
                	return this.serverData; 
                };
    }
)
.controller('cppController',
	['$scope', 	
	'srvCommands',
	function($scope, srvCommands ) {

				$scope.link = "en.wikipedia.org/";
				$scope.dataFromServer = "None";

				$scope.getData = function() {

					link = $scope.link;
					if(!link.toLowerCase().startsWith("en.wikipedia.org")){
						$scope.dataFromServer="wrong address!";
						return;
					}

					return srvCommands.getDataServer(
						function(data){
							$scope.dataFromServer=data.pozdrowienie;
						}
						,link
					);
				};
				
				
			}
	]		

			);