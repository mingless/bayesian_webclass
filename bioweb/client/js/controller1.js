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
            })


.controller('controller1', function($scope) {

	$scope.link='';
	$scope.response = 'placeholder1';
	$scope.sendWebAddress = function(){

		link = $scope.link;
		$scope.response = 'placeholder2';			
	};

})
.controller('cppController',
	['$scope', 	
	'srvCommands',
	function($scope, srvCommands ) {
				$scope.dataFromServer = 'no data!';	//to biore w htmlu

				$scope.getData = function() {
					
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