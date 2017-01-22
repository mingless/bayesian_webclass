var apk = angular.module('apk',	[]);

var link = "";
apk.service('srvCommands', //commands

	function($http) {
                this.baseURL = client_server_prefix + '/ajax/calcpy/'; //the prefix is defined in version.js
                
                this.serverData = "";
                this.getDataServer = function(callback, value) {
                	
                	this.serverData =  $http.get(this.baseURL + 'classify?word=' + value).success(callback);
                	return this.serverData; 	//standard GET method to get a response from the server
                };
    }
)
.controller('cppController',
	['$scope', 	
	'srvCommands',
	function($scope, srvCommands ) {

				$scope.link = "https://en.wikipedia.org/wiki/";
				$scope.dataFromServer = "None";

				$scope.getData = function() {

					$scope.dataFromServer = "Processing...";
					link = $scope.link;
					if(!link.startsWith("https://en.wikipedia.org/wiki/")){
						$scope.dataFromServer="Wrong address!";
						return;
					}

					return srvCommands.getDataServer(
						function(data){
							var word = data.classification.split("\n");
							var token = word[0]+ " " + " kategoria:" + word[1];
							$scope.dataFromServer=token;
						}
						,link
					);
				};
				
				
			}
	]		

			);