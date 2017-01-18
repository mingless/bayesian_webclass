
var apk = angular.module('apk',	[]);
apk
.controller('cppController',
		['$scope', 'srvCommands',
			function($scope, srvCommands) {
				$scope.dataFromServer = 'no data!';

				$scope.getData = function() {
				
					srvCommands.communicateWithServer(
						function(data){
							$scope.dataFromServer=data.toString();
						}
					);
				};
			
			}
		]		

);

