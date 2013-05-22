/* This file is part of the UAF (Unified Architecture Framework) project.
 *
 * Copyright (C) 2012 Wim Pessemier (Institute of Astronomy, KULeuven)
 *
 * Project website: http://www.ster.kuleuven.be/uaf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UAFC_SESSIONFACTORY_H_
#define UAFC_SESSIONFACTORY_H_

// STD
#include <vector>
#include <string>
#include <sstream>
// SDK
#include "uabase/uasemaphore.h"
#include "uaclient/uaclientsdk.h"
// UAF
#include "uaf/util/logger.h"
#include "uaf/client/clientexport.h"
#include "uaf/client/database/database.h"
#include "uaf/client/sessions/session.h"
#include "uaf/client/discovery/discoverer.h"
#include "uaf/client/clientinterface.h"
#include "uaf/client/requests/requests.h"
#include "uaf/client/results/results.h"
#include "uaf/client/configs/configs.h"
#include "uaf/client/invocations/invocationfactory.h"


namespace uafc
{


    /*******************************************************************************************//**
    * An uafc::SessionFactory creates and owns uafc::Session instances.
    *
    * @ingroup ClientSessions
    ***********************************************************************************************/
    class UAFC_EXPORT SessionFactory : private UaClientSdk::UaSessionCallback
    {
    public:


        /**
         * Construct a session factory.
         *
         * @param loggerFactory         Logger factory to log all messages to.
         * @param clientInterface       Interface to call when asynchronous messages are received.
         * @param discoverer            The discoverer of the client.
         * @param database              Client database to use.
         */
        SessionFactory(
                uaf::LoggerFactory* loggerFactory,
                uafc::ClientInterface* clientInterface,
                uafc::Discoverer* discoverer,
                uafc::Database* database);


        /**
         * Destruct the session factory.
         *
         * You don't have to call deleteAllSessions yourself before destroying the session factory,
         * it will be done automatically.
         */
        virtual ~SessionFactory();


        /**
         * Delete all sessions at once.
         */
        void deleteAllSessions();


        /**
         * Manually connect to a specific server.
         *
         * See uafc::Client::manuallyConnect for more info.
         *
         * @param serverUri     The server URI to which the client must connect. This server URI
         *                      (e.g. 'urn:UnifiedAutomation:UaDemoserver') must have been found
         *                      by the discovery process (which requires a Discovery URL that you
         *                      must provide via the uafc::Client::setClientSettings method).
         * @param settings      The session settings that you want your session to have.
         * @param clientConnectionId A return parameter, giving you the id of the session if it
         *                           was created.
         * @return              Good if the session was created, Bad if not.
         */
        uaf::Status manuallyConnect(
                const std::string&              serverUri,
                const uafc::SessionSettings&    settings,
                uafc::ClientConnectionId&       clientConnectionId);


        /**
         * Disconnect a session that was created manually.
         *
         * Only use this for sessions that were created via the manuallyConnect method!
         *
         * @param clientConnectionId    The id of the session (that was given as an output parameter
         *                              of the manuallyConnect method.
         * @return                      Good if the session was successfully deleted, bad if not.
         */
        uaf::Status manuallyDisconnect(uafc::ClientConnectionId clientConnectionId);


        /**
         * Do some housekeeping, such as reconnecting sessions that were disconnected, but that
         * had activities going on.
         */
        void doHouseKeeping();


        /**
         * Get some information about a particular session.
         *
         * @param clientConnectionId    The id of the session.
         * @param sessionInformation    Output parameter, giving you the information (if the
         *                              returned Status was good!).
         * @return                      Good if the clientConnectionIf was pointing to a known
         *                              (connected or disconnected) Session, or Bad if it is
         *                              pointing to a session that is not available (anymore).
         */
        uaf::Status sessionInformation(
                 uafc::ClientConnectionId   clientConnectionId,
                 uafc::SessionInformation&  sessionInformation);

        /**
         * Get the information of all sessions currently created by the client.
         *
         * These sessions may be connected, or trying to be reconnected.
         *
         * @return  A vector of the information of all sessions.
         */
        std::vector<uafc::SessionInformation> allSessionInformations();


        /**
         * Manually create a subscription.
         *
         * For more info about "manual" methods, see the documentation on the
         * uafc::Client::manuallyConnect method.
         *
         * @param clientConnectionId    The id of the session which should host the subscription.
         * @param settings              The settings of the subscription you'd like to create.
         * @param clientSubscriptionHandle  Output parameter, giving you the handle of the newly
         *                                  created subscription.
         * @return                      Good if the subscription was successfully created, Bad if
         *                              something went wrong.
         */
        uaf::Status manuallySubscribe(
                uafc::ClientConnectionId            clientConnectionId,
                const uafc::SubscriptionSettings&   settings,
                uafc::ClientSubscriptionHandle&     clientSubscriptionHandle);


        /**
         * Manually delete a subscription.
         *
         * @param clientConnectionId        The id of the session that hosts the subscription.
         * @param clientSubscriptionHandle  The handle of the subscription.
         * @return                          Good if the subscription could be deleted, bad if there
         *                                  was some problem.
         */
        uaf::Status manuallyUnsubscribe(
                uafc::ClientConnectionId        clientConnectionId,
                uafc::ClientSubscriptionHandle  clientSubscriptionHandle);


        /**
         * Get information about a particular subscription.
         *
         * @param clientSubscriptionHandle  The handle identifying the subscription.
         * @param subscriptionInformation   Output parameter, giving you the information about the
         *                                  subscription.
         * @return                          Good if the subscription could be found, Bad if not.
         */
        uaf::Status subscriptionInformation(
                 uafc::ClientSubscriptionHandle     clientSubscriptionHandle,
                 uafc::SubscriptionInformation&     subscriptionInformation);


        /**
         * Get information about all subscriptions.
         *
         * @return  A vector of all available SubscriptionInformation.
         */
        std::vector<uafc::SubscriptionInformation> allSubscriptionInformations();


        /**
         * Invoke a request.
         *
         * @tparam _Service The service to be requested (such as uafc::ReadService,
         *                  uafc::AsyncMethodCallService, etc.).
         * @param request   The request to invoke.
         * @param mask      The mask identifying the targets of the request that need to be included
         *                  in the invocation.
         * @param result    Output parameter: the result of the invocation.
         * @return          Good if the invocation went fine, bad if not.
         */
        template<typename _Service>
        uaf::Status invokeRequest(
                const typename _Service::Request&  request,
                const uaf::Mask&                   mask,
                typename _Service::Result&         result)
        {
            logger_->debug("Invoking %sRequest %d", _Service::name().c_str(), request.requestHandle());
            logger_->debug("Mask is %s", mask.toString().c_str());

            // typedef the Invocation
            typedef typename _Service::Invocation Invocation;

            // declare the return Status
            uaf::Status ret;

            // resize the result
            result.targets.resize(request.targets.size());

            // store the UAF handle and map it to a transaction id, if request is asynchronous
            TransactionId transactionId;
            bool handleStored = storeRequestHandleIfNeeded<_Service>(request, transactionId);

            // create an invocation factory
            uafc::InvocationFactory<_Service> factory;

            logger_->debug("Building the invocations");

            // build the invocations
            ret = factory.create(request, result, mask);

            logger_->debug("A total of %d invocations were built", factory.invocations.size());

            // the UAF currently does NOT support asynchronous communication to multiple sessions
            // in one request (because the logic to reconstruct the result from multiple
            // asynchronous invocations is not implemented). Synchronous communication DOES support
            // this however. So we check here if, in case of an asynchronous request, we don't have
            // more than one invocation.
            if (ret.isGood() && _Service::asynchronous)
            {
                if (factory.invocations.size() > 1)
                    ret.setStatus(uaf::statuscodes::UnsupportedError,
                           "Asynchronous requests must be able to be assigned to a single session");
            }

            // typedef the map iterator which holds the invocation for each server URI
            typedef typename std::map<std::string, Invocation*>::iterator Iter;

            // loop through the invocations (while the return Status is good)
            int invocationIndex = 0; // index to keep track of the number of processed invocations
            for (Iter it = factory.invocations.begin();
                 it != factory.invocations.end() && ret.isGood();
                 ++it)
            {
                logger_->debug("Processing invocation %d", invocationIndex);

                // create a pointer to the current invocation
                Invocation* invocation = it->second;

                // set the transactionId if necessary
                if (handleStored)
                {
                    logger_->debug("Copying the transaction id %d to the invocation",transactionId);
                    invocation->setTransactionId(transactionId);
                }

                // try to acquire a session for the current server URI and session settings
                uafc::Session* session;
                ret = acquireSession(it->first, invocation->sessionSettings(), session);

                // check if the session was acquired
                if (ret.isGood())
                {
                    // copy the session information to the invocation
                    logger_->debug("Copying the session information to the invocation");
                    invocation->setSessionInformation(session->sessionInformation());

                    // if the session is connected, invoke the service
                    if (session->isConnected() && ret.isGood())
                    {
                        logger_->debug("Forwarding the invocation to session %d",
                                       session->clientConnectionId());
                        ret = session->invokeService<_Service>(request, *invocation);
                    }
                    else
                        ret.setStatus(uaf::statuscodes::ConnectionError,
                                      "No connected session to invoke the service");

                    // copy all data to the result
                    if (!_Service::asynchronous && ret.isGood())
                    {
                        logger_->debug("Copying the invocation data to the result");
                        ret = invocation->copyToResult(result);
                    }

                    releaseSession(session);
                }

                invocationIndex++;
            }

            // remove the handle if one was stored, and if there was an unexpected error
            if (ret.isNotGood() && handleStored)
            {
                logger_->debug("Removing the transaction id");
                // remove the transaction id : request handle pair
                transactionMapMutex_.lock();            // lock the map
                transactionMap_.erase(transactionId);   // erase the key
                transactionMapMutex_.unlock();          // unlock the map
            }

            return ret;
        }


    private:


        DISALLOW_COPY_AND_ASSIGN(SessionFactory);


        // define a dataype for the number of activities associated with a specific session
        typedef uint32_t Activity;

        // define a map to store all sessions, and their number of running activities
        typedef std::map<uafc::ClientConnectionId, uafc::Session*>   SessionMap;
        typedef std::map<uafc::ClientConnectionId, Activity>         ActivityMap;

        // define a map to relate transaction ids with request handles
        typedef std::map<TransactionId, uafc::RequestHandle>  TransactionMap;


        /**
         * Acquire a session with the given properties (by getting an existing one, or creating
         * a new one if a suitable one doesn't exist already).
         *
         * The 'session' pointer can be used safely as long as releaseSession() is not called
         * by the same thread.
         *
         * @param serverUri         Server URI to create the session to.
         * @param sessionSettings   Settings of the session to be acquired.
         * @param session           Pointer to the requested session.
         * @return                  Status object, will be erroneous in case no connected session
         *                          could be provided via the 'session' argument.
         */
        uaf::Status acquireSession(
                const std::string&              serverUri,
                const uafc::SessionSettings&    sessionSettings,
                uafc::Session*&                 session);


        /**
         * Acquire an existing session with the given client connection ID.
         *
         * The 'session' pointer can be used safely as long as releaseSession() is not called
         * by the same thread.
         *
         * @param clientConnectionId    Connection id (as assigned by the client) to be found.
         * @param session               Pointer to the requested session.
         * @return                      Status object, will be erroneous in case no existing session
         *                              could be provided via the 'session' argument.
         */
        uaf::Status acquireExistingSession(
                uafc::ClientConnectionId    clientConnectionId,
                uafc::Session*&             session);


        /**
         * Release the session, so it can be garbage collected if necessary.
         *
         * Only call this function if the session was acquired, i.e. only if the status as returned
         * by acquireSession() was good!
         * After this function has been called, it is no longer safe to use the session pointer!
         *
         * @param session   Pointer to the session which we like to release.
         * @return          Good unless the session could not be released because it's already
         *                  fully released (i.e. its semaphore-like activity count is already zero).
         *                  This means that a programming bug is present, most likely in the
         *                  sessionfactory!
         */
        uaf::Status releaseSession(uafc::Session*& session, bool allowGarbageCollection=true);


        /**
         * Get a new transaction id
         *
         * @return  A unique transaction ID.
         */
        TransactionId getNewTransactionId();


        /**
         * Called every time the connection status has changed,
         * overridden from UaSessionCallback.
         */
        virtual void connectionStatusChanged(
            OpcUa_UInt32                        clientConnectionId,
            UaClientSdk::UaClient::ServerStatus serverStatus);


        /**
         * Called every time an asynchronous method call is received,
         * overridden from UaSessionCallback.
         *
         * @param transactionId     Client defined transction id for the call.
         * @param status            Overall call result.
         * @param callResponse      Output arguments and input argument results.
         */
        virtual void callComplete(
            OpcUa_UInt32                transactionId,
            const UaStatus&             status,
            const UaClientSdk::CallOut& callResponse
            );


        /**
         * Send read results.
         */
        virtual void readComplete(
            OpcUa_UInt32             transactionId,     //!< [in] Client defined transction id for the read
            const UaStatus&          uaStatus,          //!< [in] Overall read result
            const UaDataValues&      values,            //!< [in] List of read results contained in OpcUa_DataValue structures
            const UaDiagnosticInfos& diagnosticInfos    //!< [in] List of diagnostic information
            );

        /**
         * Send write results.
         */
        virtual void writeComplete(
            OpcUa_UInt32             transactionId,     //!< [in] Client defined transction id for the write
            const UaStatus&          uaStatus,          //!< [in] Overall write result
            const UaStatusCodeArray& results,           //!< [in] List of write results
            const UaDiagnosticInfos& diagnosticInfos    //!< [in] List of diagnostic information
            );


        /**
         * Generate a new transaction ID and store the request id of the associated request, if
         * necessary (i.e. if the service is asynchronous).
         *
         * As can be seen from the method signature, only Session Requests will be handled by this
         * method, so only their request handles will be stored. Subscription Requests on the other
         * hand will not be handles at this level (the session level) but by the next level
         * (the subscription level). The latter will store the request handles of asynchronous
         * subscription requests.
         *
         * @param request       The request for which we will store the request handle, if needed.
         * @param transactionId Output parameter: the newly generated transaction id, if the return
         *                      value is true.
         * @return              True if a request handle was stored, false if not.
         */
        template<typename _Service>
        bool storeRequestHandleIfNeeded(
                const uafc::BaseSessionRequest<typename _Service::Config,
                                               typename _Service::RequestTarget,
                                               _Service::asynchronous>& request,
                TransactionId& transactionId)
        {
            bool stored;

            if (_Service::asynchronous)
            {
                transactionMapMutex_.lock();
                transactionId = getNewTransactionId();
                transactionMap_[transactionId] = request.requestHandle();
                transactionMapMutex_.unlock();
                stored = true;
                logger_->debug("A new transaction id %d was stored for request %d",
                               transactionId, request.requestHandle());
            }
            else
            {
                stored = false;
                logger_->debug("Synchronous request, no transaction id needed");
            }

            return stored;
        }


        /**
         * Dummy method to catch the subscription requests, which never need to have their request
         * handles stored at this level (the session level).
         */
        template<typename _Service>
        bool storeRequestHandleIfNeeded(
                const uafc::BaseSubscriptionRequest<typename _Service::Config,
                                                    typename _Service::RequestTarget,
                                                    _Service::asynchronous>& request,
                TransactionId& transactionId)
        {
            // nothing to do
            bool stored = false;
            logger_->debug("Request must be handled at the subscription level, no transaction id "
                           "must be assigned at the session level");
            return stored;
        }


        // logger of the session factory
        uaf::Logger* logger_;
        // pointer to the client database
        uafc::Database* database_;
        // the discoverer of the servers in the system
        uafc::Discoverer* discoverer_;
        // the client interface to call whenever an asynchronous message is received
        uafc::ClientInterface* clientInterface_;

        // the current transaction id, and a mutex to safely increment it
        TransactionId   transactionId_;
        UaMutex         transactionIdMutex_;

        // the map to store the transacton ids and the associated request handles
        TransactionMap transactionMap_;
        UaMutex        transactionMapMutex_;

        // map storing all sessions
        SessionMap sessionMap_;
        // mutex to safely manipulate the sessionMap_
        UaMutex  sessionMapMutex_;

        // map storing all activity counts
        ActivityMap activityMap_;
        // mutex to safely manipulate the activity map
        UaMutex activityMapMutex_;



    };


}

#endif /* UAFC_SESSIONFACTORY_H_ */